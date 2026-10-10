#include "Tools/Baker/Baker.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>

#include "Base/Result.h"
#include "Tools/Baker/AnimMath.h"
#include "Tools/Baker/BakerUtil.h"
#include "Tools/Baker/LegacyFixups.h"
#include "Tools/Baker/BmpReader.h"
#include "Tools/Baker/JsonWriter.h"
#include "Tools/Baker/PngWriter.h"
#include "Tools/Baker/SheetSource.h"

namespace slug::baker
{

namespace
{
    using NameIndexMap = std::map<String, size_t>;

    [[nodiscard]] bool IsSolidAt_(
        const Image&           _img,
        const GroundThreshold& _th,
        const int              _x,
        const int              _y)
    {
        if (_x < 0 || _y < 0 || _x >= _img.width || _y >= _img.height)
        {
            return false;
        }
        const uint8_t* p = _img.At(_x, _y);
        return IsSolidPixel(p[0], p[1], p[2], p[3], _th);
    }

    [[nodiscard]] bool InRect_(
        const int _x,
        const int _y,
        const int _refX,
        const int _refY,
        const int _w,
        const int _h)
    {
        return _x >= _refX && _y >= _refY && _x < _refX + _w && _y < _refY + _h;
    }

    // 도로 윗면(floorY) 위로 솟은 봉우리 한 덩어리: 시드에서 4-이웃으로 번지는 고체 픽셀 중 y < floorY 인 것.
    // 사각형 (_refX, _refY, _w, _h) 안만 본다. 반환: _w*_h 비트 (1 = 봉우리). 시드가 조건을 만족하지 않으면 빈 벡터.
    [[nodiscard]] Vector<uint8_t> BumpFootprint_(
        const Image&           _img,
        const GroundThreshold& _th,
        const int              _refX,
        const int              _refY,
        const int              _w,
        const int              _h,
        const int              _floorY,
        const int              _seedX,
        const int              _seedY)
    {
        if (!InRect_(_seedX, _seedY, _refX, _refY, _w, _h) || _seedY >= _floorY || !IsSolidAt_(_img, _th, _seedX, _seedY))
        {
            return {};
        }

        constexpr ARRAY<int, 4> kDx = { 1, -1, 0, 0 };
        constexpr ARRAY<int, 4> kDy = { 0, 0, 1, -1 };

        Vector<uint8_t> bits(static_cast<size_t>(_w) * static_cast<size_t>(_h), 0u);
        bits[static_cast<size_t>(_seedY - _refY) * static_cast<size_t>(_w) + static_cast<size_t>(_seedX - _refX)] = 1u;

        Vector<std::pair<int, int>> stack;
        stack.emplace_back(_seedX, _seedY);
        while (!stack.empty())
        {
            const std::pair<int, int> cur = stack.back();
            const int                 x   = cur.first;
            const int                 y   = cur.second;
            stack.pop_back();
            for (size_t i = 0; i < kDx.size(); ++i)
            {
                const int nx = x + kDx[i];
                const int ny = y + kDy[i];
                if (!InRect_(nx, ny, _refX, _refY, _w, _h) || ny >= _floorY || !IsSolidAt_(_img, _th, nx, ny))
                {
                    continue;
                }
                uint8_t& v = bits[static_cast<size_t>(ny - _refY) * static_cast<size_t>(_w) + static_cast<size_t>(nx - _refX)];
                if (v == 0u)
                {
                    v = 1u;
                    stack.emplace_back(nx, ny);
                }
            }
        }
        return bits;
    }

    struct TextureJob
    {
        const TextureRecord* pRecord   = nullptr;
        Vector<size_t>       anims     = {};
        Vector<size_t>       sprites   = {};
        TextureStat          stat      = {};
        Vector<Anomaly>      anomalies = {};
    };

    [[nodiscard]] bool HasWeaponToken_(
        const String& _name,
        const String& _token)
    {
        const String needle = "_" + _token;
        for (size_t pos = _name.find(needle); pos != String::npos; pos = _name.find(needle, pos + 1))
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
        const String& _fileName,
        const String& _animName)
    {
        const String lower = detail::LowerAscii(_fileName);

        if (lower.starts_with("player"))
        {
            if (_animName.find("Lower") != String::npos)
            {
                return eAnimCategory::PlayerLower;
            }
            if (HasWeaponToken_(_animName, "SG") || HasWeaponToken_(_animName, "MG"))
            {
                return eAnimCategory::PlayerUpper;
            }
            return eAnimCategory::Player;
        }
        if (lower.starts_with("enemy"))
        {
            return eAnimCategory::Enemy;
        }
        if (lower.starts_with("ect") || lower.starts_with("effect"))
        {
            return eAnimCategory::Effect;
        }
        if (lower.starts_with("npc"))
        {
            return eAnimCategory::Npc;
        }
        return eAnimCategory::Other;
    }

    [[nodiscard]] int AnimFileRank_(const String& _fileName)
    {
        const String lower = detail::LowerAscii(_fileName);
        if (lower.starts_with("player"))
        {
            return 0;
        }
        if (lower.starts_with("enemy"))
        {
            return 1;
        }
        if (lower.starts_with("ect"))
        {
            return 2;
        }
        if (lower.starts_with("npc"))
        {
            return 3;
        }
        return 4;
    }

    [[nodiscard]] Vector<FilePath> ListFiles_(
        const FilePath& _dir,
        const String&   _extension)
    {
        Vector<FilePath> files;
        std::error_code  ec;
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

    struct GroundSource
    {
        const char* source = nullptr;
        const char* key    = nullptr;
    };

    constexpr ARRAY<GroundSource, 2> kGroundSources = {
        GroundSource { "Stage0_Pixel_0", "Stage0_0" },
        GroundSource { "Stage0_Pixel_1", "Stage0_1" }
    };

    class Baker
    {
    public:
        explicit Baker(
            const BakerOptions& _options);

        Baker(const Baker&)            = delete;
        Baker(Baker&&)                 = delete;
        Baker& operator=(const Baker&) = delete;
        Baker& operator=(Baker&&)      = delete;

        [[nodiscard]] slug::Result<void> Run();

    private:
        [[nodiscard]] slug::Result<void> LoadDefinitions_();
        void                             BuildJobs_();
        [[nodiscard]] TextureJob*        JobFor_(NameIndexMap& _jobIndex, const String& _texture);
        void                             ProcessJob_(TextureJob& _job);
        void                             ProcessJobAt_(size_t _index);
        void                             FailJob_(TextureJob& _job, const String& _reason);
        void                             MergeUniques_();
        [[nodiscard]] slug::Result<void> PackAndWriteAtlas_();
        void                             EncodeAtlasPageAt_(size_t _index);
        [[nodiscard]] slug::Result<void> WriteJsons_();
        [[nodiscard]] slug::Result<void> BakeSounds_();
        void                             BakeExtraSounds_(std::error_code& _ec);
        void                             BakeExtraSprites_();
        void                             BakeGrounds_();
        void                             BakeGroundAt_(size_t _index);
        void                             BakeGroundPatchMasks_();
        [[nodiscard]] int                AddUnique_(Image&& _image);
        void                             AddAnomaly_(
                                        const String& _type,
                                        const String& _name,
                                        const String& _detail,
                                        double        _value = 0.0);

        [[nodiscard]] FilePath ResourceDataDir_() const;

        const BakerOptions& m_options;
        BakeState           m_state = {};
        std::mutex          m_anomalyMutex;

        Vector<TextureRecord>          m_textures     = {};
        NameIndexMap                   m_textureIndex = {};
        Vector<TextureJob>             m_jobs         = {};
        HashMap<uint64_t, Vector<int>> m_uniqueByHash = {};

        Vector<Image>      m_pageImages = {};
        std::atomic<bool>  m_bPageFailed { false };
        FilePath           m_groundEditsPath = {};
        Vector<GroundEdit> m_groundEdits     = {};
    };

    Baker::Baker(
        const BakerOptions& _options)
        : m_options(_options)
    {
    }

    void Baker::AddAnomaly_(
        const String& _type,
        const String& _name,
        const String& _detail,
        const double  _value)
    {
        std::scoped_lock lock(m_anomalyMutex);
        m_state.anomalies.push_back({ _type, _name, _detail, _value });
    }

    FilePath Baker::ResourceDataDir_() const
    {
        return m_options.legacyRoot / "ResourceData";
    }

    slug::Result<void> Baker::LoadDefinitions_()
    {
        const FilePath dataDir = ResourceDataDir_();
        if (!fs::is_directory(dataDir))
        {
            Log("ERROR: legacy ResourceData not found: {}", dataDir.string());
            return slug::Error { slug::eError::NotFound };
        }

        {
            slug::Result<String> text = ReadText(dataDir / "TextureData.data");
            if (!text)
            {
                Log("ERROR: cannot read TextureData.data");
                return slug::Error { slug::eError::FileOpen };
            }
            for (TextureRecord& rec: ParseTextureData(*text, m_state.parseWarnings))
            {
                if (const NameIndexMap::const_iterator it = m_textureIndex.find(rec.name); it != m_textureIndex.end())
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

        NameIndexMap  animIndex;
        RbSet<String> referencedTextures;
        {
            Vector<FilePath> files = ListFiles_(dataDir / "Animation", ".anime");
            std::stable_sort(files.begin(), files.end(), [](const FilePath& _a, const FilePath& _b) {
                return AnimFileRank_(_a.filename().string()) < AnimFileRank_(_b.filename().string());
            });

            for (const FilePath& file: files)
            {
                slug::Result<String> text = ReadText(file);
                if (!text)
                {
                    Log("WARN: cannot read {}", file.string());
                    continue;
                }

                const String fileName = file.filename().string();
                for (AnimRecord& rec: ParseAnimeData(*text, fileName, m_state.parseWarnings))
                {
                    referencedTextures.insert(rec.texture);

                    // 레거시 오타 보정 (표: LegacyFixups.h). 무엇을 고쳤는지 리포트에 남긴다.
                    if (const LegacyAnimFixup* pFixup = ApplyLegacyAnimFixup(rec))
                    {
                        AddAnomaly_("LegacyAnimFixup", rec.name, pFixup->reason);
                    }

                    AnimOut out;
                    out.category = CategoryOf_(fileName, rec.name);
                    out.rec      = std::move(rec);

                    if (!m_options.Wants(CategoryFilterKey(out.category)))
                    {
                        continue;
                    }

                    if (const NameIndexMap::const_iterator it = animIndex.find(out.rec.name); it != animIndex.end())
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

        NameIndexMap spriteIndex;
        {
            for (const FilePath& file: ListFiles_(dataDir / "Sprite", ".sprite"))
            {
                slug::Result<String> text = ReadText(file);
                if (!text)
                {
                    Log("WARN: cannot read {}", file.string());
                    continue;
                }

                const String fileName = file.filename().string();
                for (SpriteRecord& rec: ParseSpriteData(*text, fileName, m_state.parseWarnings))
                {
                    referencedTextures.insert(rec.texture);
                    if (!m_options.Wants("sprites"))
                    {
                        continue;
                    }

                    SpriteOut out;
                    out.rec = std::move(rec);
                    if (const NameIndexMap::const_iterator it = spriteIndex.find(out.rec.name); it != spriteIndex.end())
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
                        out.rec.name          = tex.name;
                        out.rec.texture       = tex.name;
                        out.bImplicit         = true;
                        spriteIndex[tex.name] = m_state.sprites.size();
                        m_state.sprites.push_back(std::move(out));
                    }
                }
            }
        }

        Log("definitions: {} textures, {} animations, {} sprites", m_textures.size(), m_state.anims.size(), m_state.sprites.size());
        return slug::Error { slug::kOK };
    }

    TextureJob* Baker::JobFor_(
        NameIndexMap& _jobIndex,
        const String& _texture)
    {
        const NameIndexMap::const_iterator texIt = m_textureIndex.find(_texture);
        if (texIt == m_textureIndex.end())
        {
            return nullptr;
        }
        const std::pair<NameIndexMap::iterator, bool> inserted = _jobIndex.try_emplace(_texture, m_jobs.size());
        if (inserted.second)
        {
            m_jobs.push_back({});
            m_jobs.back().pRecord = &m_textures[texIt->second];
        }
        return &m_jobs[inserted.first->second];
    }

    void Baker::BuildJobs_()
    {
        NameIndexMap jobIndex;

        m_jobs.reserve(m_textures.size());

        for (size_t i = 0; i < m_state.anims.size(); ++i)
        {
            AnimOut& anim = m_state.anims[i];
            if (TextureJob* pJob = JobFor_(jobIndex, anim.rec.texture))
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
            if (TextureJob* pJob = JobFor_(jobIndex, sprite.rec.texture))
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

        slug::Result<Image> loaded = LoadBmpFile(m_options.legacyRoot / FilePath(tex.path));
        if (!loaded)
        {
            FailJob_(_job, "cannot load bitmap '" + tex.path + "' (" + loaded.GetError().MakeMessage() + ")");
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
                uint64_t       nonZeroAlpha = 0;
                uint64_t       artPixels    = 0;   // 마젠타가 아니고 RGB 가 0 이 아닌 픽셀
                uint64_t       artAlphaZero = 0;   // 그 중 알파가 0 인 픽셀
                const uint8_t* p            = img.pixels.data();
                for (size_t i = 0, n = img.pixels.size(); i < n; i += 4)
                {
                    const int a = p[i + 3];
                    if (a != 0)
                    {
                        ++nonZeroAlpha;
                    }
                    if (a == 0 && (p[i] | p[i + 1] | p[i + 2]) != 0)
                    {
                        ++stat.hiddenColor;
                    }
                    if (a > 0 && a < 255)
                    {
                        ++stat.partialAlpha;
                        if (std::max({ static_cast<int>(p[i]), static_cast<int>(p[i + 1]), static_cast<int>(p[i + 2]) }) > a)
                        {
                            ++stat.nonPremult;
                        }
                    }
                    if (a == 255 && p[i] == 255 && p[i + 1] == 0 && p[i + 2] == 255)
                    {
                        ++stat.opaqueMagenta;
                    }
                    else if ((p[i] | p[i + 1] | p[i + 2]) != 0)
                    {
                        ++artPixels;
                        if (a == 0)
                        {
                            ++artAlphaZero;
                        }
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
                const int   col  = r.start + i;
                const IRect cell = { col * out.cellW, r.line * out.cellH, out.cellW, out.cellH };
                FrameOut&   f    = out.frames[static_cast<size_t>(i)];

                if (cell.x + cell.w > img.width || cell.y + cell.h > img.height)
                {
                    f.bOutOfBounds = true;
                    ++outOfBounds;
                }

                const IRect bounds = FindOpaqueBounds(img, cell, m_options.alphaThreshold);
                if (bounds.IsEmpty())
                {
                    f.bEmpty          = true;
                    const Pivot pivot = ComputePivot(static_cast<uint32_t>(out.cellW), static_cast<uint32_t>(out.cellH), r.offsetX, r.offsetY, 0, 0);
                    f.pivotX          = pivot.x;
                    f.pivotY          = pivot.y;
                    continue;
                }

                f.trimX = bounds.x - cell.x;
                f.trimY = bounds.y - cell.y;
                f.w     = bounds.w;
                f.h     = bounds.h;

                const Pivot pivot                   = ComputePivot(static_cast<uint32_t>(out.cellW), static_cast<uint32_t>(out.cellH), r.offsetX, r.offsetY, f.trimX, f.trimY);
                f.pivotX                            = pivot.x;
                f.pivotY                            = pivot.y;
                out.trimmed[static_cast<size_t>(i)] = CropImage(img, bounds);
                if (ConvertSolidSilhouetteToFlash(out.trimmed[static_cast<size_t>(i)]))
                {
                    _job.anomalies.push_back({ "SolidSilhouetteFrame", r.name, "frame " + std::to_string(i) + " is a flat single-colour silhouette; baked as soft flash", 0.0 });
                }
                else if (ConvertBlackSilhouetteToWhite(out.trimmed[static_cast<size_t>(i)]))
                {
                    _job.anomalies.push_back({ "BlackSilhouetteFrame", r.name, "frame " + std::to_string(i) + " is a solid black silhouette; baked as white flash", 0.0 });
                }
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
                    Vector<uint8_t> png;
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

    void Baker::FailJob_(
        TextureJob&   _job,
        const String& _reason)
    {
        _job.stat.error = _reason;
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
    }

    void Baker::ProcessJobAt_(
        const size_t _index)
    {
        ProcessJob_(m_jobs[_index]);
    }

    int Baker::AddUnique_(Image&& _image)
    {
        const uint64_t hash   = HashImage(_image);
        Vector<int>&   bucket = m_uniqueByHash[hash];
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
            for (const FilePath& old: ListFiles_(m_options.outRoot / "Atlas", ".png"))
            {
                fs::remove(old, ec);
            }
        }

        Vector<PackSize> sizes;
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

        m_pageImages.assign(static_cast<size_t>(packed.pageCount), Image {});
        for (Image& page: m_pageImages)
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
            CopyInto(m_pageImages[static_cast<size_t>(p.page)], m_state.uniques[i], p.x, p.y);
        }

        m_bPageFailed = false;
        for (int i = 0; i < packed.pageCount; ++i)
        {
            m_state.atlasPages.push_back(std::format("Atlas/atlas_{:03d}.png", i));
        }
        ParallelFor(m_pageImages.size(), m_options.threads, *this, &Baker::EncodeAtlasPageAt_);
        Vector<Image>().swap(m_pageImages);
        if (m_bPageFailed)
        {
            Log("ERROR: failed to write an atlas page");
            return slug::Error { slug::eError::FileOpen };
        }

        const double capacity = static_cast<double>(packed.pageCount) * m_options.pageSize * m_options.pageSize;
        Log("atlas: {} unique images ({} frames total), {} page(s) {}x{}, fill {:.1f}%", m_state.uniques.size(), m_state.totalFrames, packed.pageCount, m_options.pageSize, m_options.pageSize, capacity > 0 ? 100.0 * static_cast<double>(m_state.uniquePixels) / capacity : 0.0);
        return slug::Error { slug::kOK };
    }

    void Baker::EncodeAtlasPageAt_(
        const size_t _index)
    {
        Vector<uint8_t> png;
        if (!EncodePng(m_pageImages[_index], png) || !WriteBytes(m_options.outRoot / m_state.atlasPages[_index], png.data(), png.size()))
        {
            m_bPageFailed = true;
        }
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
                    const Pivot          pivot = s.bCustomPivot ? Pivot { s.pivotX, s.pivotY } : ComputePivot(static_cast<uint32_t>(s.cellW), static_cast<uint32_t>(s.cellH), 0, 0, 0, 0);
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
            for (const String& page: m_state.atlasPages)
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

        slug::Result<String> text = ReadText(ResourceDataDir_() / "Sound" / "SoundData.txt");
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

            const FilePath src = ResolveCaseInsensitive(m_options.legacyRoot / FilePath(rec.path));
            if (!fs::is_regular_file(src, ec))
            {
                out.error = "missing sound file: " + rec.path;
                AddAnomaly_("MissingSound", rec.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }

            const String fileName = src.filename().string();
            out.file              = "Audio/" + fileName;
            fs::copy_file(src, m_options.outRoot / out.file, fs::copy_options::overwrite_existing, ec);
            if (ec)
            {
                out.error = "copy failed: " + ec.message();
                AddAnomaly_("CopyFailed", rec.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }

            const String lower = detail::LowerAscii(rec.name);
            out.bBgm           = lower.find("bgm") != String::npos;
            out.bOk            = true;
            m_state.sounds.push_back(std::move(out));
        }

        BakeExtraSounds_(ec);

        size_t ok = 0;
        for (const SoundOut& s: m_state.sounds)
        {
            ok += s.bOk ? 1 : 0;
        }
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
            bool        bBgm = false;
        };
        static constexpr ARRAY<ExtraSound, 9> kExtras = {
            ExtraSound { "Sfx_GameOver", "Sounds/NARRATOR MS1,2/sfx-gameover.mp3", "Sfx_GameOver.mp3", false },
            ExtraSound {  "Sfx_PowerUp",  "Sounds/NARRATOR MS1,2/sfx-powerup.mp3",  "Sfx_PowerUp.mp3", false },
            ExtraSound {    "Sfx_Bingo",    "Sounds/NARRATOR MS1,2/sfx-bingo.mp3",    "Sfx_Bingo.mp3", false },
            ExtraSound {   "Sfx_LetsGo",   "Sounds/NARRATOR MS1,2/sfx-letsgo.mp3",   "Sfx_LetsGo.mp3", false },
            ExtraSound { "Sfx_HereWeGo", "Sounds/NARRATOR MS1,2/sfx-herewego.mp3", "Sfx_HereWeGo.mp3", false },
            ExtraSound {      "Sfx_Yah",      "Sounds/NARRATOR MS1,2/sfx-yah.mp3",      "Sfx_Yah.mp3", false },
            ExtraSound {    "Sfx_YeeHa",    "Sounds/NARRATOR MS1,2/sfx-yeeha.mp3",    "Sfx_YeeHa.mp3", false },
            ExtraSound {    "Title_MS3",        "Sounds/MS X,3,4,5/MS3_Title.ogg",    "Title_MS3.ogg", false },
            ExtraSound {    "Title_MS2",    "Sounds/NARRATOR MS1,2/MS2_Title.ogg",    "Title_MS2.ogg", false },
        };

        for (const ExtraSound& extra: kExtras)
        {
            SoundOut out;
            out.name = extra.name;

            const FilePath src = ResolveCaseInsensitive(m_options.legacyRoot / FilePath(extra.legacyPath));
            if (!fs::is_regular_file(src, _ec))
            {
                out.error = String("missing sound file: ") + extra.legacyPath;
                AddAnomaly_("MissingSound", out.name, out.error);
                m_state.sounds.push_back(std::move(out));
                continue;
            }

            out.file = String("Audio/") + extra.outFile;
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

        slug::Result<Image> loaded = LoadBmpFile(m_options.legacyRoot / FilePath("Dummy/Main_Title.bmp"));
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
                ARRAY<uint32_t, 4> sum = { 0, 0, 0, 0 };
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

        Vector<uint8_t> png;
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

        m_state.grounds.resize(kGroundSources.size());

        // 지형 편집 (GroundEdits.json): 굽기 전에 원본 비트맵 사각형을 공기/고체로 바꾼다 (재현 가능한 파이프라인)
        m_groundEditsPath = m_options.groundEditsPath;
        if (m_groundEditsPath.empty())
        {
            m_groundEditsPath = m_options.outRoot.parent_path() / "Source" / "GroundEdits.json";
        }
        slug::Result<Vector<GroundEdit>> edits = LoadGroundEdits(m_groundEditsPath);
        if (!edits)
        {
            Log("WARN: ground edits failed: {}", m_groundEditsPath.string());
            AddAnomaly_("GroundEditsFailed", m_groundEditsPath.string(), "invalid GroundEdits.json");
            edits = Vector<GroundEdit> {};
        }
        m_groundEdits = *edits;
        ParallelFor(kGroundSources.size(), m_options.threads, *this, &Baker::BakeGroundAt_);

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

    void Baker::BakeGroundAt_(
        const size_t _index)
    {
        GroundOut& out = m_state.grounds[_index];
        out.source     = kGroundSources[_index].source;
        out.key        = kGroundSources[_index].key;
        out.file       = String("Ground/") + out.key + ".bin";

        String           path = String("Stage/map/") + out.source + ".bmp";
        eTransparentMode mode = eTransparentMode::Alpha;
        if (const NameIndexMap::const_iterator it = m_textureIndex.find(out.source); it != m_textureIndex.end())
        {
            path = m_textures[it->second].path;
            mode = m_textures[it->second].mode;
        }

        slug::Result<Image> loaded = LoadBmpFile(m_options.legacyRoot / FilePath(path));
        if (!loaded)
        {
            out.error = "cannot load " + path;
            return;
        }
        Image img = loaded.Take();
        if (mode == eTransparentMode::None)
        {
            ForceOpaque(img);
        }
        if (const int applied = ApplyGroundEdits(img, m_groundEdits, out.source))
        {
            Log("ground {}: {} pixel(s) edited ({})", out.key, applied, m_groundEditsPath.string());
        }

        out.w                      = img.width;
        out.h                      = img.height;
        const Vector<uint8_t> bits = BuildGroundBits(img, m_options.ground, &out.stats);
        const Vector<uint8_t> file = BuildGroundFile(img.width, img.height, bits);
        if (!WriteBytes(m_options.outRoot / out.file, file.data(), file.size()))
        {
            out.error = "cannot write " + out.file;
            return;
        }
        out.bOk = true;
    }

    void Baker::BakeGroundPatchMasks_()
    {
        if (!m_options.Wants("ground"))
        {
            return;
        }

        // 지면 패치 마스크 = 파괴 시 공기가 되는 발자국. 파란 픽셀(alpha 255) = 지울 자리, 나머지는 투명.
        // 런타임 ApplyGroundPatch 는 이 자리만 지운다 (Docs/PropSpecs.md §2 groundPatch).
        //   - Stage0_House_5: 레거시 마스크는 '집 몸체를 뺀 지면' 스냅샷이다. 기준 지면과의 차이 = 집 몸체 윗부분.
        //   - Stage0_House_4: 레거시 마스크는 전부 흰색(빈 마스크)이라 쓸 수 없다. 도로 윗면(floorY) 위로 솟은
        //     발자국 중심 열의 봉우리 한 덩어리 (4-이웃 연결) 을 기준 지면에서 직접 잡는다.
        // refX/refY = 런타임 좌상단 (PropWorld 의 center - (size/2 - inset), 실수면 floor). 기준 인스턴스는 Stage0.json 의 첫 배치.
        struct PatchMaskSpec
        {
            const char* sprite;
            int         w               = 0;
            int         h               = 0;
            int         refX            = 0;
            int         refY            = 0;
            bool        bLegacySnapshot = false;
            int         floorY          = 0;
            int         seedX           = 0;
            int         seedY           = 0;
        };
        static constexpr ARRAY<PatchMaskSpec, 2> kSpecs = {
            PatchMaskSpec { "Stage0_House_5_PixelMap",  555, 124, 10204, 1204,  true,    0,    0,    0 },
            PatchMaskSpec { "Stage0_House_4_PixelMap", 1306, 751,  9189,  898, false, 1286, 9830, 1219 },
        };

        slug::Result<Image> baseLoaded = LoadBmpFile(m_options.legacyRoot / FilePath("Stage/map/Stage0_Pixel_0.bmp"));
        if (!baseLoaded)
        {
            Log("WARN: ground patch masks skipped: cannot load Stage/map/Stage0_Pixel_0.bmp");
            AddAnomaly_("GroundPatchFailed", "Stage0_Pixel_0", "cannot load Stage/map/Stage0_Pixel_0.bmp");
            return;
        }
        const Image base = baseLoaded.Take();

        for (const PatchMaskSpec& spec: kSpecs)
        {
            Image    mask      = Image::Make(spec.w, spec.h);
            uint64_t footprint = 0;

            if (spec.bLegacySnapshot)
            {
                slug::Result<Image> snapLoaded = LoadBmpFile(m_options.legacyRoot / FilePath(String("Stage/map/") + spec.sprite + ".bmp"));
                if (!snapLoaded)
                {
                    Log("WARN: ground patch mask {} skipped: legacy snapshot missing", spec.sprite);
                    AddAnomaly_("GroundPatchFailed", spec.sprite, "legacy snapshot missing");
                    continue;
                }
                const Image snap = snapLoaded.Take();
                if (snap.width != spec.w || snap.height != spec.h)
                {
                    Log("WARN: ground patch mask {} skipped: legacy snapshot size mismatch", spec.sprite);
                    AddAnomaly_("GroundPatchFailed", spec.sprite, "legacy snapshot size mismatch");
                    continue;
                }
                for (int y = 0; y < spec.h; ++y)
                {
                    for (int x = 0; x < spec.w; ++x)
                    {
                        if (IsSolidAt_(base, m_options.ground, spec.refX + x, spec.refY + y) && !IsSolidAt_(snap, m_options.ground, x, y))
                        {
                            uint8_t* p = mask.At(x, y);
                            p[0]       = 0;
                            p[1]       = 0;
                            p[2]       = 255;
                            p[3]       = 255;
                            ++footprint;
                        }
                    }
                }
            }
            else
            {
                const Vector<uint8_t> bits = BumpFootprint_(base, m_options.ground, spec.refX, spec.refY, spec.w, spec.h, spec.floorY, spec.seedX, spec.seedY);
                if (bits.empty())
                {
                    Log("WARN: ground patch mask {} skipped: seed is not a solid pixel above the floor", spec.sprite);
                    AddAnomaly_("GroundPatchFailed", spec.sprite, "seed is not a solid pixel above the floor");
                    continue;
                }
                for (int y = 0; y < spec.h; ++y)
                {
                    for (int x = 0; x < spec.w; ++x)
                    {
                        if (bits[static_cast<size_t>(y) * static_cast<size_t>(spec.w) + static_cast<size_t>(x)] != 0u)
                        {
                            uint8_t* p = mask.At(x, y);
                            p[0]       = 0;
                            p[1]       = 0;
                            p[2]       = 255;
                            p[3]       = 255;
                            ++footprint;
                        }
                    }
                }
            }

            if (footprint == 0)
            {
                Log("WARN: ground patch mask {} has an empty footprint; not written", spec.sprite);
                AddAnomaly_("GroundPatchFailed", spec.sprite, "empty footprint");
                continue;
            }

            Vector<uint8_t> png;
            const String    file = String("Textures/") + spec.sprite + ".png";
            if (!EncodePng(mask, png) || !WriteBytes(m_options.outRoot / file, png.data(), png.size()))
            {
                AddAnomaly_("WriteFailed", spec.sprite, file);
                continue;
            }
            Log("ground patch {}: footprint {} px ({}x{})", spec.sprite, footprint, spec.w, spec.h);
        }
    }

    slug::Result<void> Baker::Run()
    {
        const SteadyClock::time_point start = SteadyClock::now();

        std::error_code                 ec;
        constexpr ARRAY<const char*, 5> kSubDirs = { "Atlas", "Textures", "Ground", "Audio", "Reports" };
        for (const char* pSub: kSubDirs)
        {
            fs::create_directories(m_options.outRoot / pSub, ec);
        }

        if (slug::Result<void> r = LoadDefinitions_(); !r)
        {
            return r;
        }
        BuildJobs_();

        Log("processing {} textures ...", m_jobs.size());
        ParallelFor(m_jobs.size(), m_options.threads, *this, &Baker::ProcessJobAt_);

        for (TextureJob& job: m_jobs)
        {
            for (Anomaly& a: job.anomalies)
            {
                m_state.anomalies.push_back(std::move(a));
            }
            m_state.textures.push_back(job.stat);
        }

        if (slug::Result<void> r = AppendSheetClips(m_options, m_state); !r)
        {
            return r;
        }

        size_t validAnims = 0;
        for (const AnimOut& a: m_state.anims)
        {
            if (a.bValid)
            {
                ++validAnims;
            }
            else
            {
                Log("SKIP anim {}: {}", a.rec.name, a.skipReason);
            }
        }
        for (const SpriteOut& s: m_state.sprites)
        {
            if (!s.bValid)
            {
                Log("SKIP sprite {}: {}", s.rec.name, s.skipReason);
            }
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
        BakeGroundPatchMasks_();

        if (slug::Result<void> r = WriteJsons_(); !r)
        {
            return r;
        }

        m_state.elapsedSec = std::chrono::duration<double>(SteadyClock::now() - start).count();
        return WriteReports(m_options, m_state);
    }
}   // namespace

slug::Result<void> RunBaker(const BakerOptions& _options)
{
    if (!_options.sheetPreview.empty())
    {
        return RunSheetPreview(_options);
    }
    Baker baker(_options);
    return baker.Run();
}

}   // namespace slug::baker
