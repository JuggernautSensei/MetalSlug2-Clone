#include "Tools/Baker/Baker.h"
#include "Base/Result.h"

#include <algorithm>
#include <chrono>
#include <map>
#include <unordered_map>

#include "Tools/Baker/AnimMath.h"
#include "Tools/Baker/BakerUtil.h"
#include "Tools/Baker/BmpReader.h"
#include "Tools/Baker/JsonWriter.h"
#include "Tools/Baker/PngWriter.h"

namespace slug::baker
{

namespace fs = std::filesystem;

namespace
{
    struct TextureJob
    {
        const TextureRecord*  pRecord = nullptr;
        std::vector<size_t>   anims;
        std::vector<size_t>   sprites;
        TextureStat           stat;
        std::vector<Anomaly>  anomalies;
    };

    [[nodiscard]] bool HasWeaponToken_(
        const std::string& _name,
        const std::string& _token)
    {
        const std::string needle = "_" + _token;
        for (size_t pos = _name.find(needle); pos != std::string::npos; pos = _name.find(needle, pos + 1))
        {
            const size_t end = pos + needle.size();
            if (end == _name.size() || _name[end] == '_')
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] eAnimCategory CategoryOf_(
        const std::string& _fileName,
        const std::string& _animName)
    {
        std::string lower = _fileName;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](const unsigned char _c) { return static_cast<char>(std::tolower(_c)); });

        if (lower.starts_with("player"))
        {
            if (_animName.find("Lower") != std::string::npos)
            {
                return eAnimCategory::PlayerLower;
            }
            if (HasWeaponToken_(_animName, "SG") || HasWeaponToken_(_animName, "MG"))
            {
                return eAnimCategory::PlayerUpper;
            }
            return eAnimCategory::Player;
        }
        if (lower.starts_with("enemy")) { return eAnimCategory::Enemy; }
        if (lower.starts_with("ect") || lower.starts_with("effect")) { return eAnimCategory::Effect; }
        if (lower.starts_with("npc")) { return eAnimCategory::Npc; }
        return eAnimCategory::Other;
    }

    [[nodiscard]] int AnimFileRank_(const std::string& _fileName)
    {
        std::string lower = _fileName;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](const unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
        if (lower.starts_with("player")) { return 0; }
        if (lower.starts_with("enemy")) { return 1; }
        if (lower.starts_with("ect")) { return 2; }
        if (lower.starts_with("npc")) { return 3; }
        return 4;
    }

    [[nodiscard]] std::vector<fs::path> ListFiles_(
        const fs::path&    _dir,
        const std::string& _extension)
    {
        std::vector<fs::path> files;
        std::error_code       ec;
        for (const fs::directory_entry& entry: fs::directory_iterator(_dir, ec))
        {
            if (entry.is_regular_file() && entry.path().extension() == _extension)
            {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    class Baker
    {
    public:
        explicit Baker(const BakerOptions& _options)
            : m_options(_options)
        {
        }

        [[nodiscard]] slug::Result<void> Run();

    private:
        [[nodiscard]] slug::Result<void> LoadDefinitions_();
        void                            BuildJobs_();
        void                            ProcessJob_(TextureJob& _job);
        void                            MergeUniques_();
        [[nodiscard]] slug::Result<void> PackAndWriteAtlas_();
        [[nodiscard]] slug::Result<void> WriteJsons_();
        [[nodiscard]] slug::Result<void> BakeSounds_();
        void                            BakeExtraSounds_(std::error_code& _ec);
        void                            BakeExtraSprites_();
        void                            BakeGrounds_();
        [[nodiscard]] int               AddUnique_(Image&& _image);

        void AddAnomaly_(const std::string& _type, const std::string& _name, const std::string& _detail, double _value = 0.0)
        {
            std::scoped_lock lock(m_anomalyMutex);
            m_state.anomalies.push_back({ _type, _name, _detail, _value });
        }

        [[nodiscard]] fs::path ResourceDataDir_() const { return m_options.legacyRoot / "ResourceData"; }

        const BakerOptions& m_options;
        BakeState           m_state;
        std::mutex          m_anomalyMutex;

        std::vector<TextureRecord>             m_textures;
        std::map<std::string, size_t>          m_textureIndex;
        std::vector<TextureJob>                m_jobs;
        std::unordered_map<uint64_t, std::vector<int>> m_uniqueByHash;
    };

    slug::Result<void> Baker::LoadDefinitions_()
    {
        const fs::path dataDir = ResourceDataDir_();
        if (!fs::is_directory(dataDir))
        {
            Log("ERROR: legacy ResourceData not found: {}", dataDir.string());
            return slug::Error { slug::eError::NotFound };
        }

        {
            slug::Result<std::string> text = ReadText(dataDir / "TextureData.data");
            if (!text)
            {
                Log("ERROR: cannot read TextureData.data");
                return slug::Error { slug::eError::FileOpen };
            }
            for (TextureRecord& rec: ParseTextureData(*text, m_state.parseWarnings))
            {
                if (const auto it = m_textureIndex.find(rec.name); it != m_textureIndex.end())
                {
                    AddAnomaly_("DuplicateTexture", rec.name, "later TextureData entry overrides the earlier one");
                    m_textures[it->second] = std::move(rec);
                }
                else
                {
                    m_textureIndex[rec.name] = m_textures.size();
                    m_textures.push_back(std::move(rec));
                }
            }
        }
        m_state.textureCount = m_textures.size();

        std::map<std::string, size_t> animIndex;
        std::set<std::string>         referencedTextures;
        {
            std::vector<fs::path> files = ListFiles_(dataDir / "Animation", ".anime");
            std::stable_sort(files.begin(), files.end(), [](const fs::path& _a, const fs::path& _b)
            {
                return AnimFileRank_(_a.filename().string()) < AnimFileRank_(_b.filename().string());
            });

            for (const fs::path& file: files)
            {
                slug::Result<std::string> text = ReadText(file);
                if (!text)
                {
                    Log("WARN: cannot read {}", file.string());
                    continue;
                }

                const std::string fileName = file.filename().string();
                for (AnimRecord& rec: ParseAnimeData(*text, fileName, m_state.parseWarnings))
                {
                    referencedTextures.insert(rec.texture);

                    AnimOut out;
                    out.category = CategoryOf_(fileName, rec.name);
                    out.rec      = std::move(rec);

                    if (!m_options.Wants(CategoryFilterKey(out.category)))
                    {
                        continue;
                    }

                    if (const auto it = animIndex.find(out.rec.name); it != animIndex.end())
                    {
                        AddAnomaly_("DuplicateName", out.rec.name, "animation defined twice (" + m_state.anims[it->second].rec.sourceFile + " then " + fileName + "); later one wins");
                        m_state.anims[it->second] = std::move(out);
                    }
                    else
                    {
                        animIndex[out.rec.name] = m_state.anims.size();
                        m_state.anims.push_back(std::move(out));
                    }
                }
            }
        }

        std::map<std::string, size_t> spriteIndex;
        {
            for (const fs::path& file: ListFiles_(dataDir / "Sprite", ".sprite"))
            {
                slug::Result<std::string> text = ReadText(file);
                if (!text)
                {
                    Log("WARN: cannot read {}", file.string());
                    continue;
                }

                const std::string fileName = file.filename().string();
                for (SpriteRecord& rec: ParseSpriteData(*text, fileName, m_state.parseWarnings))
                {
                    referencedTextures.insert(rec.texture);
                    if (!m_options.Wants("sprites"))
                    {
                        continue;
                    }

                    SpriteOut out;
                    out.rec = std::move(rec);
                    if (const auto it = spriteIndex.find(out.rec.name); it != spriteIndex.end())
                    {
                        AddAnomaly_("DuplicateName", out.rec.name, "sprite defined twice; later one wins");
                        m_state.sprites[it->second] = std::move(out);
                    }
                    else
                    {
                        spriteIndex[out.rec.name] = m_state.sprites.size();
                        m_state.sprites.push_back(std::move(out));
                    }
                }
            }

            if (m_options.Wants("sprites"))
            {
                for (const TextureRecord& tex: m_textures)
                {
                    if (!referencedTextures.contains(tex.name) && !spriteIndex.contains(tex.name))
                    {
                        SpriteOut out;
                        out.rec.name    = tex.name;
                        out.rec.texture = tex.name;
                        out.bImplicit   = true;
                        spriteIndex[tex.name] = m_state.sprites.size();
                        m_state.sprites.push_back(std::move(out));
                    }
                }
            }
        }

        Log("definitions: {} textures, {} animations, {} sprites", m_textures.size(), m_state.anims.size(), m_state.sprites.size());
        return slug::Error { slug::kOK };
    }

    void Baker::BuildJobs_()
    {
        std::map<std::string, size_t> jobIndex;
        auto                          jobFor = [&](const std::string& _texture) -> TextureJob*
        {
            const auto texIt = m_textureIndex.find(_texture);
            if (texIt == m_textureIndex.end())
            {
                return nullptr;
            }
            const auto [it, bInserted] = jobIndex.try_emplace(_texture, m_jobs.size());
            if (bInserted)
            {
                m_jobs.push_back({});
                m_jobs.back().pRecord = &m_textures[texIt->second];
            }
            return &m_jobs[it->second];
        };

        m_jobs.reserve(m_textures.size());

        for (size_t i = 0; i < m_state.anims.size(); ++i)
        {
            AnimOut& anim = m_state.anims[i];
            if (TextureJob* pJob = jobFor(anim.rec.texture))
            {
                pJob->anims.push_back(i);
            }
            else
            {
                anim.skipReason = "texture not defined in TextureData: " + anim.rec.texture;
                AddAnomaly_("MissingTexture", anim.rec.name, anim.skipReason);
            }
        }
        for (size_t i = 0; i < m_state.sprites.size(); ++i)
        {
            SpriteOut& sprite = m_state.sprites[i];
            if (TextureJob* pJob = jobFor(sprite.rec.texture))
            {
                pJob->sprites.push_back(i);
            }
            else
            {
                sprite.skipReason = "texture not defined in TextureData: " + sprite.rec.texture;
                AddAnomaly_("MissingTexture", sprite.rec.name, sprite.skipReason);
            }
        }
    }

    void Baker::ProcessJob_(TextureJob& _job)
    {
        const TextureRecord& tex  = *_job.pRecord;
        TextureStat&         stat = _job.stat;
        stat.name                 = tex.name;

        auto fail = [&](const std::string& _reason)
        {
            stat.error = _reason;
            for (const size_t i: _job.anims)
            {
                m_state.anims[i].skipReason = _reason;
                _job.anomalies.push_back({ "MissingFile", m_state.anims[i].rec.name, _reason, 0.0 });
            }
            for (const size_t i: _job.sprites)
            {
                m_state.sprites[i].skipReason = _reason;
                _job.anomalies.push_back({ "MissingFile", m_state.sprites[i].rec.name, _reason, 0.0 });
            }
        };

        slug::Result<Image> loaded = LoadBmpFile(m_options.legacyRoot / fs::path(tex.path));
        if (!loaded)
        {
            fail("cannot load bitmap '" + tex.path + "' (" + loaded.GetError().MakeMessage() + ")");
            return;
        }
        Image img = loaded.Take();

        stat.bOk             = true;
        stat.w               = img.width;
        stat.h               = img.height;
        stat.bSourceHasAlpha = img.bSourceHasAlpha;

        switch (tex.mode)
        {
            case eTransparentMode::None: ForceOpaque(img); break;
            case eTransparentMode::ColorKey: ApplyColorKey(img, 255, 0, 255); break;
            case eTransparentMode::Alpha:
            {
                uint64_t nonZeroAlpha = 0;
                uint64_t artPixels    = 0;   // 마젠타가 아니고 RGB 가 0 이 아닌 픽셀
                uint64_t artAlphaZero = 0;   // 그 중 알파가 0 인 픽셀
                const uint8_t* p      = img.pixels.data();
                for (size_t i = 0, n = img.pixels.size(); i < n; i += 4)
                {
                    const int a = p[i + 3];
                    if (a != 0) { ++nonZeroAlpha; }
                    if (a == 0 && (p[i] | p[i + 1] | p[i + 2]) != 0) { ++stat.hiddenColor; }
                    if (a > 0 && a < 255)
                    {
                        ++stat.partialAlpha;
                        if (std::max({ int(p[i]), int(p[i + 1]), int(p[i + 2]) }) > a) { ++stat.nonPremult; }
                    }
                    if (a == 255 && p[i] == 255 && p[i + 1] == 0 && p[i + 2] == 255) { ++stat.opaqueMagenta; }
                    else if ((p[i] | p[i + 1] | p[i + 2]) != 0)
                    {
                        ++artPixels;
                        if (a == 0) { ++artAlphaZero; }
                    }
                }

                const bool bJunkAlpha = stat.opaqueMagenta > 0 && artPixels > 0 && artAlphaZero * 2 > artPixels;
                if (m_options.bKeyMagentaInAlpha && bJunkAlpha)
                {
                    ApplyColorKey(img, 255, 0, 255);
                    _job.anomalies.push_back({ "AlphaChannelUnreliable", tex.name, "UseTransparent=2 but art pixels have alpha 0 and background is opaque magenta; baked as magenta colorkey", static_cast<double>(artAlphaZero) });
                }
                else if (m_options.bKeyMagentaInAlpha && stat.opaqueMagenta > 0)
                {
                    ApplyMagentaKeyKeepAlpha(img);
                    _job.anomalies.push_back({ "MagentaKeyedInAlphaTexture", tex.name, std::to_string(stat.opaqueMagenta) + " opaque (255,0,255) pixels treated as transparent although UseTransparent=2", static_cast<double>(stat.opaqueMagenta) });
                }

                if (!img.bSourceHasAlpha)
                {
                    _job.anomalies.push_back({ "AlphaModeWithoutAlpha", tex.name, "UseTransparent=2 but bitmap has no alpha channel; treated as opaque", 0.0 });
                }
                else if (nonZeroAlpha == 0)
                {
                    _job.anomalies.push_back({ "AlphaAllZero", tex.name, "UseTransparent=2 but every alpha is 0; treated as opaque", 0.0 });
                    ForceOpaque(img);
                }
                break;
            }
        }
        ClearTransparentColor(img);

        for (const size_t animIdx: _job.anims)
        {
            AnimOut&          out = m_state.anims[animIdx];
            const AnimRecord& r   = out.rec;

            if (r.width <= 0 || r.height <= 0 || r.start < 0 || r.end < r.start || r.line < 0)
            {
                out.skipReason = "invalid range/size (Start=" + std::to_string(r.start) + " End=" + std::to_string(r.end) + " W=" + std::to_string(r.width) + " H=" + std::to_string(r.height) + ")";
                _job.anomalies.push_back({ "InvalidAnimation", r.name, out.skipReason, 0.0 });
                continue;
            }

            out.cellW = ResolveCellWidth(img.width, r.width, r.end);
            out.cellH = r.height;
            if (out.cellW <= 0)
            {
                out.skipReason = "cell width resolved to 0";
                _job.anomalies.push_back({ "InvalidAnimation", r.name, out.skipReason, 0.0 });
                continue;
            }

            const int count = r.end - r.start + 1;
            out.frames.resize(static_cast<size_t>(count));
            out.trimmed.resize(static_cast<size_t>(count));
            int      outOfBounds = 0;
            uint64_t magentaPx   = 0;

            for (int i = 0; i < count; ++i)
            {
                const int    col  = r.start + i;
                const IRect  cell = { col * out.cellW, r.line * out.cellH, out.cellW, out.cellH };
                FrameOut&    f    = out.frames[static_cast<size_t>(i)];

                if (cell.x + cell.w > img.width || cell.y + cell.h > img.height)
                {
                    f.bOutOfBounds = true;
                    ++outOfBounds;
                }

                const IRect bounds = FindOpaqueBounds(img, cell, m_options.alphaThreshold);
                if (bounds.IsEmpty())
                {
                    f.bEmpty = true;
                    const Pivot pivot = ComputePivot(static_cast<uint32_t>(out.cellW), static_cast<uint32_t>(out.cellH), r.offsetX, r.offsetY, 0, 0);
                    f.pivotX          = pivot.x;
                    f.pivotY          = pivot.y;
                    continue;
                }

                f.trimX = bounds.x - cell.x;
                f.trimY = bounds.y - cell.y;
                f.w     = bounds.w;
                f.h     = bounds.h;

                const Pivot pivot = ComputePivot(static_cast<uint32_t>(out.cellW), static_cast<uint32_t>(out.cellH), r.offsetX, r.offsetY, f.trimX, f.trimY);
                f.pivotX          = pivot.x;
                f.pivotY          = pivot.y;
                out.trimmed[static_cast<size_t>(i)] = CropImage(img, bounds);
                magentaPx += CountOpaqueMagenta(out.trimmed[static_cast<size_t>(i)]);
            }

            if (magentaPx > 0)
            {
                _job.anomalies.push_back({ "OpaqueMagentaInFrame", r.name, std::to_string(magentaPx) + " opaque (255,0,255) pixels inside baked frames of texture '" + tex.name + "'", static_cast<double>(magentaPx) });
            }

            if (outOfBounds > 0)
            {
                _job.anomalies.push_back({ "FrameOutOfBounds", r.name, std::to_string(outOfBounds) + " of " + std::to_string(count) + " frame cells exceed the texture (" + std::to_string(img.width) + "x" + std::to_string(img.height) + ", cell " + std::to_string(out.cellW) + "x" + std::to_string(out.cellH) + ")", static_cast<double>(outOfBounds) });
            }
            out.bValid = true;
        }

        bool bStandaloneWritten = false;
        for (const size_t spriteIdx: _job.sprites)
        {
            SpriteOut&          out = m_state.sprites[spriteIdx];
            const SpriteRecord& r   = out.rec;

            out.texW  = img.width;
            out.texH  = img.height;
            out.cellW = (r.width >= 0) ? r.width : img.width;
            out.cellH = (r.height >= 0) ? r.height : img.height;
            out.col   = r.column;
            out.line  = r.line;
            if (out.cellW <= 0 || out.cellH <= 0 || out.col < 0 || out.line < 0)
            {
                out.skipReason = "invalid sprite cell";
                _job.anomalies.push_back({ "InvalidSprite", r.name, out.skipReason, 0.0 });
                continue;
            }

            const IRect cell = { out.col * out.cellW, out.line * out.cellH, out.cellW, out.cellH };
            if (cell.x + cell.w > img.width || cell.y + cell.h > img.height)
            {
                _job.anomalies.push_back({ "SpriteOutOfBounds", r.name, "cell exceeds texture (" + std::to_string(img.width) + "x" + std::to_string(img.height) + ")", 0.0 });
            }

            if (std::max(out.cellW, out.cellH) > m_options.standaloneSize)
            {
                out.bFile = true;
                out.file  = "Textures/" + SanitizeFileName(tex.name) + ".png";
                if (!bStandaloneWritten)
                {
                    std::vector<uint8_t> png;
                    if (!EncodePng(img, png) || !WriteBytes(m_options.outRoot / out.file, png.data(), png.size()))
                    {
                        out.skipReason = "failed to write standalone PNG";
                        _job.anomalies.push_back({ "WriteFailed", r.name, out.skipReason, 0.0 });
                        continue;
                    }
                    bStandaloneWritten = true;
                    stat.bStandalone   = true;
                }
            }
            else
            {
                out.image = CropImage(img, cell);
                if (const uint64_t magentaPx = CountOpaqueMagenta(out.image); magentaPx > 0)
                {
                    _job.anomalies.push_back({ "OpaqueMagentaInFrame", r.name, std::to_string(magentaPx) + " opaque (255,0,255) pixels inside sprite cell", static_cast<double>(magentaPx) });
                }
            }
            out.bValid = true;
        }
    }

    int Baker::AddUnique_(Image&& _image)
    {
        const uint64_t hash = HashImage(_image);
        std::vector<int>& bucket = m_uniqueByHash[hash];
        for (const int index: bucket)
        {
            if (SameImage(m_state.uniques[static_cast<size_t>(index)], _image))
            {
                return index;
            }
        }

        const int index = static_cast<int>(m_state.uniques.size());
        m_state.uniquePixels += static_cast<uint64_t>(_image.width) * static_cast<uint64_t>(_image.height);
        m_state.uniques.push_back(std::move(_image));
        bucket.push_back(index);
        return index;
    }

    void Baker::MergeUniques_()
    {
        for (AnimOut& anim: m_state.anims)
        {
            if (!anim.bValid)
            {
                continue;
            }
            for (size_t i = 0; i < anim.frames.size(); ++i)
            {
                ++m_state.totalFrames;
                if (!anim.frames[i].bEmpty)
                {
                    anim.frames[i].unique = AddUnique_(std::move(anim.trimmed[i]));
                }
            }
            anim.trimmed.clear();
            anim.trimmed.shrink_to_fit();
        }

        for (SpriteOut& sprite: m_state.sprites)
        {
            if (sprite.bValid && !sprite.bFile)
            {
                sprite.unique = AddUnique_(std::move(sprite.image));
                sprite.image  = {};
            }
        }
    }

    slug::Result<void> Baker::PackAndWriteAtlas_()
    {
        {
            std::error_code ec;
            for (const fs::path& old: ListFiles_(m_options.outRoot / "Atlas", ".png"))
            {
                fs::remove(old, ec);
            }
        }

        std::vector<PackSize> sizes;
        sizes.reserve(m_state.uniques.size());
        for (const Image& img: m_state.uniques)
        {
            sizes.push_back({ img.width, img.height });
        }

        const PackResult packed = PackShelf(sizes, m_options.pageSize, m_options.padding);
        for (size_t i = 0; i < packed.placements.size(); ++i)
        {
            if (!packed.placements[i].bPlaced)
            {
                Log("ERROR: image {}x{} does not fit a {}px atlas page", sizes[i].w, sizes[i].h, m_options.pageSize);
                return slug::Error { slug::eError::InvalidData };
            }
        }
        m_state.placements = packed.placements;
        m_state.pageCount  = packed.pageCount;

        std::vector<Image> pages(static_cast<size_t>(packed.pageCount));
        for (Image& page: pages)
        {
            page = Image::Make(m_options.pageSize, m_options.pageSize);
        }
        for (size_t i = 0; i < m_state.uniques.size(); ++i)
        {
            const PackPlacement& p = packed.placements[i];
            if (m_state.uniques[i].IsEmpty())
            {
                continue;
            }
            CopyInto(pages[static_cast<size_t>(p.page)], m_state.uniques[i], p.x, p.y);
        }

        std::atomic<bool> bFailed { false };
        for (int i = 0; i < packed.pageCount; ++i)
        {
            m_state.atlasPages.push_back(std::format("Atlas/atlas_{:03d}.png", i));
        }
        ParallelFor(pages.size(), m_options.threads, [&](const size_t _i)
        {
            std::vector<uint8_t> png;
            if (!EncodePng(pages[_i], png) || !WriteBytes(m_options.outRoot / m_state.atlasPages[_i], png.data(), png.size()))
            {
                bFailed = true;
            }
        });
        if (bFailed)
        {
            Log("ERROR: failed to write an atlas page");
            return slug::Error { slug::eError::FileOpen };
        }

        const double capacity = static_cast<double>(packed.pageCount) * m_options.pageSize * m_options.pageSize;
        Log("atlas: {} unique images ({} frames total), {} page(s) {}x{}, fill {:.1f}%", m_state.uniques.size(), m_state.totalFrames, packed.pageCount, m_options.pageSize, m_options.pageSize, capacity > 0 ? 100.0 * static_cast<double>(m_state.uniquePixels) / capacity : 0.0);
        return slug::Error { slug::kOK };
    }

    slug::Result<void> Baker::WriteJsons_()
    {
        {
            JsonWriter w;
            w.BeginObject();
            w.Field("version", 1);
            w.Key("animations");
            w.BeginObject();
            for (const AnimOut& anim: m_state.anims)
            {
                if (!anim.bValid)
                {
                    continue;
                }
                const AnimRecord& r = anim.rec;
                w.Key(r.name);
                w.BeginObject();
                w.Field("loop", r.bLoop);
                w.Field("loopStart", r.bLoop ? r.optionIndex : 0);
                if (!r.bLoop && r.optionIndex > 0)
                {
                    w.Field("cancelFrame", std::max(0, r.optionIndex - r.start));
                }
                w.Field("frameDuration", r.frameDuration);
                w.Field("cellW", anim.cellW);
                w.Field("cellH", anim.cellH);
                w.Key("frames");
                w.BeginArray();
                for (const FrameOut& f: anim.frames)
                {
                    w.BeginObject(true);
                    if (f.bEmpty)
                    {
                        w.Field("atlas", 0);
                        w.Field("x", 0);
                        w.Field("y", 0);
                    }
                    else
                    {
                        const PackPlacement& p = m_state.placements[static_cast<size_t>(f.unique)];
                        w.Field("atlas", p.page);
                        w.Field("x", p.x);
                        w.Field("y", p.y);
                    }
                    w.Field("w", f.w);
                    w.Field("h", f.h);
                    w.Field("pivotX", f.pivotX);
                    w.Field("pivotY", f.pivotY);
                    w.EndObject();
                }
                w.EndArray();
                w.EndObject();
            }
            w.EndObject();
            w.EndObject();
            if (slug::Result<void> r = WriteText(m_options.outRoot / "anim.json", w.Str()); !r)
            {
                return r;
            }
        }

        {
            JsonWriter w;
            w.BeginObject();
            w.Field("version", 1);
            w.Key("sprites");
            w.BeginObject();
            for (const SpriteOut& s: m_state.sprites)
            {
                if (!s.bValid)
                {
                    continue;
                }
                w.Key(s.rec.name);
                w.BeginObject(true);
                if (s.bFile)
                {
                    w.Field("file", s.file);
                    w.Field("w", s.texW);
                    w.Field("h", s.texH);
                    w.Field("columns", std::max(1, s.texW / s.cellW));
                    w.Field("rows", std::max(1, s.texH / s.cellH));
                    w.Field("cellW", s.cellW);
                    w.Field("cellH", s.cellH);
                    w.Field("col", s.col);
                    w.Field("line", s.line);
                }
                else
                {
                    const PackPlacement& p     = m_state.placements[static_cast<size_t>(s.unique)];
                    const Pivot          pivot = ComputePivot(static_cast<uint32_t>(s.cellW), static_cast<uint32_t>(s.cellH), 0, 0, 0, 0);
                    w.Field("atlas", p.page);
                    w.Field("x", p.x);
                    w.Field("y", p.y);
                    w.Field("w", s.cellW);
                    w.Field("h", s.cellH);
                    w.Field("pivotX", pivot.x);
                    w.Field("pivotY", pivot.y);
                }
                w.EndObject();
            }
            w.EndObject();
            w.EndObject();
            if (slug::Result<void> r = WriteText(m_options.outRoot / "sprites.json", w.Str()); !r)
            {
                return r;
            }
        }

        {
            JsonWriter w;
            w.BeginObject();
            w.Field("version", 1);
            w.Key("sounds");
            w.BeginObject();
            for (const SoundOut& s: m_state.sounds)
            {
                if (!s.bOk)
                {
                    continue;
                }
                w.Key(s.name);
                w.BeginObject(true);
                w.Field("file", s.file);
                w.Field("bgm", s.bBgm);
                w.EndObject();
            }
            w.EndObject();
            w.EndObject();
            if (slug::Result<void> r = WriteText(m_options.outRoot / "sounds.json", w.Str()); !r)
            {
                return r;
            }
        }

        {
            JsonWriter w;
            w.BeginObject();
            w.Field("version", 1);
            w.Key("atlasPages");
            w.BeginArray(true);
            for (const std::string& page: m_state.atlasPages)
            {
                w.Value(page);
            }
            w.EndArray();
            w.Field("animations", "anim.json");
            w.Field("sprites", "sprites.json");
            w.Field("sounds", "sounds.json");
            w.Key("ground");
            w.BeginObject(true);
            for (const GroundOut& g: m_state.grounds)
            {
                if (g.bOk)
                {
                    w.Field(g.key, g.file);
                }
            }
            w.EndObject();
            w.EndObject();
            if (slug::Result<void> r = WriteText(m_options.outRoot / "manifest.json", w.Str()); !r)
            {
                return r;
            }
        }

        return slug::Error { slug::kOK };
    }

    slug::Result<void> Baker::BakeSounds_()
    {
        if (!m_options.Wants("sounds"))
        {
            return slug::Error { slug::kOK };
        }

        slug::Result<std::string> text = ReadText(ResourceDataDir_() / "Sound" / "SoundData.txt");
        if (!text)
        {
            Log("WARN: SoundData.txt not found; skipping sounds");
            return slug::Error { slug::kOK };
        }

        std::error_code ec;
        fs::create_directories(m_options.outRoot / "Audio", ec);

        for (const SoundRecord& rec: ParseSoundData(*text, m_state.parseWarnings))
        {
            SoundOut out;
            out.name = rec.name;

            const fs::path src = m_options.legacyRoot / fs::path(rec.path);
            if (!fs::is_regular_file(src, ec))
            {
                out.error = "missing sound file: " + rec.path;
                AddAnomaly_("MissingSound", rec.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }

            const std::string fileName = src.filename().string();
            out.file                   = "Audio/" + fileName;
            fs::copy_file(src, m_options.outRoot / out.file, fs::copy_options::overwrite_existing, ec);
            if (ec)
            {
                out.error = "copy failed: " + ec.message();
                AddAnomaly_("CopyFailed", rec.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }

            std::string lower = rec.name;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](const unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
            out.bBgm = lower.find("bgm") != std::string::npos;
            out.bOk  = true;
            m_state.sounds.push_back(std::move(out));
        }

        BakeExtraSounds_(ec);

        size_t ok = 0;
        for (const SoundOut& s: m_state.sounds) { ok += s.bOk ? 1 : 0; }
        Log("sounds: {} of {} copied", ok, m_state.sounds.size());
        return slug::Error { slug::kOK };
    }

    void Baker::BakeExtraSounds_(
        std::error_code& _ec)
    {
        struct ExtraSound
        {
            const char* name;
            const char* legacyPath;
            const char* outFile;
            bool        bBgm;
        };
        static constexpr ExtraSound kExtras[] = {
            { "Sfx_GameOver", "Sounds/NARRATOR MS1,2/sfx-gameover.mp3", "Sfx_GameOver.mp3", false },
            { "Sfx_PowerUp", "Sounds/NARRATOR MS1,2/sfx-powerup.mp3", "Sfx_PowerUp.mp3", false },
            { "Sfx_Bingo", "Sounds/NARRATOR MS1,2/sfx-bingo.mp3", "Sfx_Bingo.mp3", false },
            { "Sfx_LetsGo", "Sounds/NARRATOR MS1,2/sfx-letsgo.mp3", "Sfx_LetsGo.mp3", false },
            { "Sfx_HereWeGo", "Sounds/NARRATOR MS1,2/sfx-herewego.mp3", "Sfx_HereWeGo.mp3", false },
            { "Sfx_Yah", "Sounds/NARRATOR MS1,2/sfx-yah.mp3", "Sfx_Yah.mp3", false },
            { "Sfx_YeeHa", "Sounds/NARRATOR MS1,2/sfx-yeeha.mp3", "Sfx_YeeHa.mp3", false },
            { "Title_MS3", "Sounds/MS X,3,4,5/MS3_Title.ogg", "Title_MS3.ogg", false },
            { "Title_MS2", "Sounds/NARRATOR MS1,2/MS2_Title.ogg", "Title_MS2.ogg", false },
        };

        for (const ExtraSound& extra: kExtras)
        {
            SoundOut out;
            out.name = extra.name;

            const fs::path src = m_options.legacyRoot / fs::path(extra.legacyPath);
            if (!fs::is_regular_file(src, _ec))
            {
                out.error = std::string("missing sound file: ") + extra.legacyPath;
                AddAnomaly_("MissingSound", out.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }

            out.file = std::string("Audio/") + extra.outFile;
            fs::copy_file(src, m_options.outRoot / out.file, fs::copy_options::overwrite_existing, _ec);
            if (_ec)
            {
                out.error = "copy failed: " + _ec.message();
                AddAnomaly_("CopyFailed", out.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }
            out.bBgm = extra.bBgm;
            out.bOk  = true;
            m_state.sounds.push_back(std::move(out));
        }
    }

    void Baker::BakeExtraSprites_()
    {
        if (!m_options.Wants("sprites"))
        {
            return;
        }

        slug::Result<Image> loaded = LoadBmpFile(m_options.legacyRoot / fs::path("Dummy/Main_Title.bmp"));
        if (!loaded)
        {
            Log("WARN: Dummy/Main_Title.bmp not found; skipping TitleBackground");
            AddAnomaly_("MissingTexture", "TitleBackground", "Dummy/Main_Title.bmp");
            return;
        }
        Image src = loaded.Take();
        ForceOpaque(src);

        const int dstW = src.width / 2;
        const int dstH = src.height / 2;
        if (dstW <= 0 || dstH <= 0)
        {
            return;
        }

        Image dst = Image::Make(dstW, dstH);
        for (int y = 0; y < dstH; ++y)
        {
            for (int x = 0; x < dstW; ++x)
            {
                uint32_t sum[4] = { 0, 0, 0, 0 };
                for (int dy = 0; dy < 2; ++dy)
                {
                    for (int dx = 0; dx < 2; ++dx)
                    {
                        const uint8_t* pSrc = src.At(x * 2 + dx, y * 2 + dy);
                        for (int c = 0; c < 4; ++c)
                        {
                            sum[c] += pSrc[c];
                        }
                    }
                }
                uint8_t* pDst = dst.At(x, y);
                for (int c = 0; c < 4; ++c)
                {
                    pDst[c] = static_cast<uint8_t>((sum[c] + 2) / 4);
                }
            }
        }

        SpriteOut out;
        out.rec.name = "TitleBackground";
        out.bValid   = true;
        out.cellW    = dstW;
        out.cellH    = dstH;
        out.texW     = dstW;
        out.texH     = dstH;
        out.bFile    = true;
        out.file     = "Textures/TitleBackground.png";

        std::vector<uint8_t> png;
        if (!EncodePng(dst, png) || !WriteBytes(m_options.outRoot / out.file, png.data(), png.size()))
        {
            Log("WARN: failed to write {}", out.file);
            AddAnomaly_("WriteFailed", "TitleBackground", out.file);
            return;
        }
        m_state.sprites.push_back(std::move(out));
    }

    void Baker::BakeGrounds_()
    {
        if (!m_options.Wants("ground"))
        {
            return;
        }

        struct GroundSource
        {
            const char* source;
            const char* key;
        };
        static constexpr GroundSource kSources[] = {
            { "Stage0_Pixel_0", "Stage0_0" },
            { "Stage0_Pixel_1", "Stage0_1" },
        };

        m_state.grounds.resize(std::size(kSources));
        ParallelFor(std::size(kSources), m_options.threads, [&](const size_t _i)
        {
            GroundOut& out = m_state.grounds[_i];
            out.source     = kSources[_i].source;
            out.key        = kSources[_i].key;
            out.file       = std::string("Ground/") + out.key + ".bin";

            std::string path = std::string("Stage/map/") + out.source + ".bmp";
            eTransparentMode mode = eTransparentMode::Alpha;
            if (const auto it = m_textureIndex.find(out.source); it != m_textureIndex.end())
            {
                path = m_textures[it->second].path;
                mode = m_textures[it->second].mode;
            }

            slug::Result<Image> loaded = LoadBmpFile(m_options.legacyRoot / fs::path(path));
            if (!loaded)
            {
                out.error = "cannot load " + path;
                return;
            }
            Image img = loaded.Take();
            if (mode == eTransparentMode::None) { ForceOpaque(img); }

            out.w = img.width;
            out.h = img.height;
            const std::vector<uint8_t> bits = BuildGroundBits(img, m_options.ground, &out.stats);
            const std::vector<uint8_t> file = BuildGroundFile(img.width, img.height, bits);
            if (!WriteBytes(m_options.outRoot / out.file, file.data(), file.size()))
            {
                out.error = "cannot write " + out.file;
                return;
            }
            out.bOk = true;
        });

        for (const GroundOut& g: m_state.grounds)
        {
            if (g.bOk)
            {
                Log("ground {}: {}x{} solid={} ({:.2f}%) unclassified={} bounds=({},{} {}x{})", g.key, g.w, g.h, g.stats.solidCount, 100.0 * static_cast<double>(g.stats.solidCount) / static_cast<double>(std::max<uint64_t>(g.stats.totalCount, 1)), g.stats.unclassifiedCount, g.stats.solidBounds.x, g.stats.solidBounds.y, g.stats.solidBounds.w, g.stats.solidBounds.h);
            }
            else
            {
                Log("WARN: ground {} failed: {}", g.key, g.error);
                AddAnomaly_("GroundFailed", g.key, g.error);
            }
        }
    }

    slug::Result<void> Baker::Run()
    {
        const auto start = std::chrono::steady_clock::now();

        std::error_code ec;
        for (const char* sub: { "Atlas", "Textures", "Ground", "Audio", "Reports" })
        {
            fs::create_directories(m_options.outRoot / sub, ec);
        }

        if (slug::Result<void> r = LoadDefinitions_(); !r)
        {
            return r;
        }
        BuildJobs_();

        Log("processing {} textures ...", m_jobs.size());
        ParallelFor(m_jobs.size(), m_options.threads, [&](const size_t _i) { ProcessJob_(m_jobs[_i]); });

        for (TextureJob& job: m_jobs)
        {
            for (Anomaly& a: job.anomalies)
            {
                m_state.anomalies.push_back(std::move(a));
            }
            m_state.textures.push_back(job.stat);
        }

        size_t validAnims = 0;
        for (const AnimOut& a: m_state.anims)
        {
            if (a.bValid) { ++validAnims; }
            else { Log("SKIP anim {}: {}", a.rec.name, a.skipReason); }
        }
        for (const SpriteOut& s: m_state.sprites)
        {
            if (!s.bValid) { Log("SKIP sprite {}: {}", s.rec.name, s.skipReason); }
        }
        Log("baked animations: {} of {}", validAnims, m_state.anims.size());

        MergeUniques_();
        BakeExtraSprites_();
        if (slug::Result<void> r = PackAndWriteAtlas_(); !r)
        {
            return r;
        }

        if (slug::Result<void> r = BakeSounds_(); !r)
        {
            return r;
        }
        BakeGrounds_();

        if (slug::Result<void> r = WriteJsons_(); !r)
        {
            return r;
        }

        m_state.elapsedSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        return WriteReports(m_options, m_state);
    }
}

slug::Result<void> RunBaker(const BakerOptions& _options)
{
    Baker baker(_options);
    return baker.Run();
}
}
