#include "Tools/Baker/Baker.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <format>
#include <map>

#include "Base/Result.h"
#include "Tools/Baker/AnimMath.h"
#include "Tools/Baker/BakerUtil.h"
#include "Tools/Baker/JsonWriter.h"
#include "Tools/Baker/PngWriter.h"

namespace slug::baker
{

namespace
{
    struct AnimAnalysis
    {
        DriftResult drift       = {};
        int         emptyFrames = 0;
        double      footY0      = 0.0;   // 첫 비어있지 않은 프레임의 바닥 앵커 Y (원점 기준)
        bool        bHasFoot    = false;
    };

    [[nodiscard]] Vector<FrameBox> BoxesOf_(const AnimOut& _anim)
    {
        Vector<FrameBox> boxes;
        boxes.reserve(_anim.frames.size());
        for (const FrameOut& f: _anim.frames)
        {
            boxes.push_back({ f.w, f.h, f.pivotX, f.pivotY, f.bEmpty });
        }
        return boxes;
    }

    [[nodiscard]] String Fmt1_(const double _v)
    {
        return std::format("{:.1f}", _v);
    }

    constexpr ARRAY<uint16_t, 10> kDigitGlyphs = {
        0b111101101101111,
        0b010110010010111,
        0b111001111100111,
        0b111001111001111,
        0b101101111001001,
        0b111100111001111,
        0b111100111101111,
        0b111001001001001,
        0b111101111101111,
        0b111101111001111
    };

    void DrawNumber_(
        Image&    _img,
        const int _x,
        const int _y,
        const int _number)
    {
        const String text = std::to_string(_number);
        int          cx   = _x;
        for (const char c: text)
        {
            const uint16_t glyph = kDigitGlyphs[static_cast<size_t>(c - '0')];
            for (int row = 0; row < 5; ++row)
            {
                for (int col = 0; col < 3; ++col)
                {
                    if ((glyph >> (14 - (row * 3 + col))) & 1)
                    {
                        PutPixel(_img, cx + col, _y + row, 255, 255, 255);
                    }
                }
            }
            cx += 4;
        }
    }

    constexpr int kLabelH    = 7;
    constexpr int kSheetMaxW = 2560;
    constexpr int kSheetMaxH = 4096;
    constexpr int kBlockGap  = 4;

    struct Block
    {
        size_t animIndex  = 0;
        size_t firstFrame = 0;
        size_t frameCount = 0;
        int    label      = 0;
        int    slotW      = 0;   // 1x 기준 슬롯 크기
        int    slotH      = 0;
        int    originX    = 0;   // 슬롯 내 원점 위치 (1x)
        int    originY    = 0;
        int    x          = 0;
        int    y          = 0;
        int    w          = 0;
        int    h          = 0;
    };

    struct SheetPlan
    {
        String        file   = {};
        Vector<Block> blocks = {};
        int           w      = 0;
        int           h      = 0;
        int           scale  = 1;
    };

    void DrawScaled_(
        Image&       _dst,
        const Image& _src,
        const int    _dx,
        const int    _dy,
        const int    _scale)
    {
        for (int y = 0; y < _src.height; ++y)
        {
            for (int x = 0; x < _src.width; ++x)
            {
                const uint8_t* s = _src.At(x, y);
                if (s[3] == 0)
                {
                    continue;
                }
                for (int sy = 0; sy < _scale; ++sy)
                {
                    for (int sx = 0; sx < _scale; ++sx)
                    {
                        const int px = _dx + x * _scale + sx;
                        const int py = _dy + y * _scale + sy;
                        if (px < 0 || py < 0 || px >= _dst.width || py >= _dst.height)
                        {
                            continue;
                        }
                        uint8_t*  d = _dst.At(px, py);
                        const int a = s[3];
                        d[0]        = static_cast<uint8_t>((s[0] * a + d[0] * (255 - a) + 127) / 255);
                        d[1]        = static_cast<uint8_t>((s[1] * a + d[1] * (255 - a) + 127) / 255);
                        d[2]        = static_cast<uint8_t>((s[2] * a + d[2] * (255 - a) + 127) / 255);
                    }
                }
            }
        }
    }

    void RenderBlock_(
        Image&           _sheet,
        const Block&     _block,
        const BakeState& _state,
        const int        _scale)
    {
        const AnimOut& anim = _state.anims[_block.animIndex];

        FillRect(_sheet, { _block.x, _block.y, _block.w, kLabelH }, 24, 24, 32);
        DrawNumber_(_sheet, _block.x + 1, _block.y + 1, _block.label);

        const int bodyY = _block.y + kLabelH;
        const int slotW = _block.slotW * _scale;
        const int slotH = _block.slotH * _scale;

        for (size_t i = 0; i < _block.frameCount; ++i)
        {
            const int sx = _block.x + static_cast<int>(i) * (slotW + 1);
            for (int y = 0; y < slotH; y += 8)
            {
                for (int x = 0; x < slotW; x += 8)
                {
                    const bool bDark = (((x / 8) + (y / 8)) & 1) == 0;
                    FillRect(_sheet, { sx + x, bodyY + y, std::min(8, slotW - x), std::min(8, slotH - y) }, bDark ? 74 : 92, bDark ? 74 : 92, bDark ? 86 : 104);
                }
            }
            if (i + 1 < _block.frameCount)
            {
                FillRect(_sheet, { sx + slotW, bodyY, 1, slotH }, 20, 20, 24);
            }
        }

        int footLineY = -1;
        for (size_t i = 0; i < _block.frameCount; ++i)
        {
            const FrameOut& f = anim.frames[_block.firstFrame + i];
            if (!f.bEmpty)
            {
                footLineY = (_block.originY + f.h - f.pivotY) * _scale;
                break;
            }
        }

        for (size_t i = 0; i < _block.frameCount; ++i)
        {
            const FrameOut& f  = anim.frames[_block.firstFrame + i];
            const int       sx = _block.x + static_cast<int>(i) * (slotW + 1);

            if (!f.bEmpty)
            {
                const Image& img = _state.uniques[static_cast<size_t>(f.unique)];
                DrawScaled_(_sheet, img, sx + (_block.originX - f.pivotX) * _scale, bodyY + (_block.originY - f.pivotY) * _scale, _scale);
            }

            if (footLineY >= 0 && footLineY < slotH)
            {
                for (int x = 0; x < slotW; ++x)
                {
                    PutPixel(_sheet, sx + x, bodyY + footLineY, 60, 230, 90);
                }
            }

            const int cx = sx + _block.originX * _scale + _scale / 2;
            const int cy = bodyY + _block.originY * _scale + _scale / 2;
            for (int d = -3; d <= 3; ++d)
            {
                PutPixel(_sheet, cx + d, cy, 255, 30, 30);
                PutPixel(_sheet, cx, cy + d, 255, 30, 30);
            }
            if (f.bEmpty)
            {
                PutPixel(_sheet, sx + 1, bodyY + 1, 255, 220, 0);
            }
        }
    }

    void AddSheetPlan_(
        Vector<SheetPlan>& _plans,
        const String&      _key,
        const int          _scale)
    {
        SheetPlan plan;
        plan.scale = _scale;
        plan.file  = _plans.empty() ? ("contact_" + _key + ".png") : std::format("contact_{}_{}.png", _key, _plans.size() + 1);
        _plans.push_back(std::move(plan));
    }

    void PlanCategory_(
        const BakeState&    _state,
        const eAnimCategory _category,
        Vector<SheetPlan>&  _plans,
        String&             _legend)
    {
        const String key   = CategoryKey(_category);
        const int    scale = (_category == eAnimCategory::Player || _category == eAnimCategory::PlayerUpper || _category == eAnimCategory::PlayerLower) ? 2 : 1;

        Vector<Block> blocks;
        int           label = 0;
        for (size_t a = 0; a < _state.anims.size(); ++a)
        {
            const AnimOut& anim = _state.anims[a];
            if (!anim.bValid || anim.category != _category)
            {
                continue;
            }
            ++label;

            int  left   = 0;
            int  right  = 0;
            int  top    = 0;
            int  bottom = 0;
            bool bAny   = false;
            for (const FrameOut& f: anim.frames)
            {
                if (f.bEmpty)
                {
                    continue;
                }
                const int l = -f.pivotX;
                const int r = f.w - f.pivotX;
                const int t = -f.pivotY;
                const int b = f.h - f.pivotY;
                if (!bAny)
                {
                    left   = l;
                    right  = r;
                    top    = t;
                    bottom = b;
                    bAny   = true;
                }
                else
                {
                    left   = std::min(left, l);
                    right  = std::max(right, r);
                    top    = std::min(top, t);
                    bottom = std::max(bottom, b);
                }
            }

            _legend += std::format("{}\t{}\t{}\tframes={}{}\n", label, anim.rec.name, key, anim.frames.size(), bAny ? "" : " (all empty)");
            if (!bAny)
            {
                continue;
            }

            left   = std::min(left, -4);
            right  = std::max(right, 5);
            top    = std::min(top, -4);
            bottom = std::max(bottom, 5);

            Block proto;
            proto.animIndex = a;
            proto.label     = label;
            proto.originX   = 2 - left;
            proto.originY   = 2 - top;
            proto.slotW     = (right - left) + 4;
            proto.slotH     = (bottom - top) + 4;

            const int    slotPx  = proto.slotW * scale + 1;
            const size_t perLine = static_cast<size_t>(std::max(1, (kSheetMaxW - 2 * kBlockGap) / slotPx));
            for (size_t first = 0; first < anim.frames.size(); first += perLine)
            {
                Block block      = proto;
                block.firstFrame = first;
                block.frameCount = std::min(perLine, anim.frames.size() - first);
                block.w          = static_cast<int>(block.frameCount) * slotPx - 1;
                block.h          = kLabelH + proto.slotH * scale;
                blocks.push_back(block);
            }
        }

        if (blocks.empty())
        {
            return;
        }

        Vector<SheetPlan> plans;
        AddSheetPlan_(plans, key, scale);

        int cx    = kBlockGap;
        int cy    = kBlockGap;
        int rowH  = 0;
        int usedW = 0;
        for (Block& block: blocks)
        {
            if (cx + block.w + kBlockGap > kSheetMaxW && cx > kBlockGap)
            {
                cx = kBlockGap;
                cy += rowH + kBlockGap;
                rowH = 0;
            }
            if (cy + block.h + kBlockGap > kSheetMaxH && cy > kBlockGap)
            {
                plans.back().w = usedW + kBlockGap;
                plans.back().h = cy + rowH + kBlockGap;
                AddSheetPlan_(plans, key, scale);
                cx    = kBlockGap;
                cy    = kBlockGap;
                rowH  = 0;
                usedW = 0;
            }

            block.x = cx;
            block.y = cy;
            plans.back().blocks.push_back(block);
            cx += block.w + kBlockGap;
            rowH  = std::max(rowH, block.h);
            usedW = std::max(usedW, cx);
        }
        plans.back().w = usedW + kBlockGap;
        plans.back().h = cy + rowH + kBlockGap;

        for (SheetPlan& plan: plans)
        {
            _plans.push_back(std::move(plan));
        }
    }

    // 접촉 시트 PNG 한 장을 렌더링해서 쓴다. ParallelFor 의 작업 단위.
    class ContactSheetWriter
    {
    public:
        ContactSheetWriter(
            const Vector<SheetPlan>& _plans,
            const BakeState&         _state,
            const FilePath&          _reportDir)
            : m_plans(_plans)
            , m_state(_state)
            , m_reportDir(_reportDir)
        {
        }

        void Write(
            const size_t _index)
        {
            const SheetPlan& plan  = m_plans[_index];
            Image            sheet = Image::Make(plan.w, plan.h);
            FillRect(sheet, { 0, 0, plan.w, plan.h }, 16, 16, 22);
            for (const Block& block: plan.blocks)
            {
                RenderBlock_(sheet, block, m_state, plan.scale);
            }

            Vector<uint8_t> png;
            if (!EncodePng(sheet, png) || !WriteBytes(m_reportDir / plan.file, png.data(), png.size()))
            {
                m_bFailed = true;
            }
        }

        [[nodiscard]] bool IsFailed() const
        {
            return m_bFailed;
        }

    private:
        const Vector<SheetPlan>& m_plans;
        const BakeState&         m_state;
        const FilePath&          m_reportDir;
        std::atomic<bool>        m_bFailed { false };
    };

    // 요약 텍스트를 한 줄씩 쌓는다.
    class SummaryWriter
    {
    public:
        void Line(
            const String& _text)
        {
            m_text += _text;
            m_text += '\n';
        }

        [[nodiscard]] const String& GetText() const
        {
            return m_text;
        }

    private:
        String m_text = {};
    };

    [[nodiscard]] String JoinOnly_(
        const RbSet<String>& _only)
    {
        String joined;
        for (const String& item: _only)
        {
            joined += item + " ";
        }
        return joined;
    }

    // 유형별 상위 이상 항목을 요약에 덧붙인다.
    void AppendTopAnomalies_(
        SummaryWriter&   _report,
        const BakeState& _state,
        const String&    _type,
        const size_t     _n)
    {
        _report.Line("");
        _report.Line(std::format("top {} (see pivot_report.json)", _type));
        size_t shown = 0;
        for (const Anomaly& a: _state.anomalies)
        {
            if (a.type == _type && shown < _n)
            {
                _report.Line(std::format("    {} : {}", a.animation, a.detail));
                ++shown;
            }
        }
    }

    using NameIndexMap = std::map<String, size_t>;
    using CountMap     = std::map<String, size_t>;

    void CheckLeftRightTwins_(
        const BakeState& _state,
        Vector<Anomaly>& _out)
    {
        NameIndexMap byName;
        for (size_t i = 0; i < _state.anims.size(); ++i)
        {
            if (_state.anims[i].bValid)
            {
                byName[_state.anims[i].rec.name] = i;
            }
        }

        const String kLeft  = "_Left";
        const String kRight = "_Right";
        for (const NameIndexMap::value_type& entry: byName)
        {
            const String& name  = entry.first;
            const size_t  index = entry.second;
            if (!name.ends_with(kLeft))
            {
                continue;
            }
            const String                       base = name.substr(0, name.size() - kLeft.size());
            const NameIndexMap::const_iterator it   = byName.find(base + kRight);
            if (it == byName.end())
            {
                _out.push_back({ "LeftRightMissingTwin", name, "no matching '" + base + kRight + "'", 0.0 });
                continue;
            }

            const AnimOut& l = _state.anims[index];
            const AnimOut& r = _state.anims[it->second];

            const int mirrorErrX = l.rec.offsetX + r.rec.offsetX;
            const int diffY      = l.rec.offsetY - r.rec.offsetY;
            if (mirrorErrX != 0 || diffY != 0)
            {
                const double magnitude = std::max(std::abs(mirrorErrX), std::abs(diffY));
                _out.push_back({ "LeftRightOffsetMismatch", base, std::format("{} Left=({},{}) Right=({},{}) mirrorErrX={} diffY={}", magnitude >= 10 ? "HIGH" : "low", l.rec.offsetX, l.rec.offsetY, r.rec.offsetX, r.rec.offsetY, mirrorErrX, diffY), magnitude });
            }

            String props;
            if (l.cellW != r.cellW || l.cellH != r.cellH)
            {
                props += std::format(" cell {}x{} vs {}x{};", l.cellW, l.cellH, r.cellW, r.cellH);
            }
            if (l.frames.size() != r.frames.size())
            {
                props += std::format(" frames {} vs {};", l.frames.size(), r.frames.size());
            }
            if (std::abs(l.rec.frameDuration - r.rec.frameDuration) > 1e-9)
            {
                props += std::format(" duration {} vs {};", l.rec.frameDuration, r.rec.frameDuration);
            }
            if (l.rec.bLoop != r.rec.bLoop)
            {
                props += " loop differs;";
            }
            if (l.rec.optionIndex != r.rec.optionIndex)
            {
                props += std::format(" optionIndex {} vs {};", l.rec.optionIndex, r.rec.optionIndex);
            }
            if (!props.empty())
            {
                _out.push_back({ "LeftRightPropertyMismatch", base, props, 1.0 });
            }

            const size_t common = std::min(l.frames.size(), r.frames.size());
            double       maxDy  = 0.0;
            for (size_t i = 0; i < common; ++i)
            {
                const FrameOut& fl = l.frames[i];
                const FrameOut& fr = r.frames[i];
                if (fl.bEmpty || fr.bEmpty)
                {
                    continue;
                }
                maxDy = std::max(maxDy, std::abs(static_cast<double>((fl.h - fl.pivotY) - (fr.h - fr.pivotY))));
            }
            if (maxDy > 1.0)
            {
                _out.push_back({ "LeftRightFootMismatch", base, std::format("bottom anchor Y differs by up to {} px between Left and Right", Fmt1_(maxDy)), maxDy });
            }
        }
    }
}   // namespace

slug::Result<void> WriteReports(
    const BakerOptions& _options,
    BakeState&          _state)
{
    const FilePath  reportDir = _options.outRoot / "Reports";
    std::error_code ec;
    fs::create_directories(reportDir, ec);

    Vector<AnimAnalysis> analysis(_state.anims.size());
    for (size_t i = 0; i < _state.anims.size(); ++i)
    {
        const AnimOut& anim = _state.anims[i];
        if (!anim.bValid)
        {
            continue;
        }

        AnimAnalysis& a = analysis[i];
        a.drift         = ComputeAnchorDrift(BoxesOf_(anim), anim.rec.bLoop, anim.rec.optionIndex);
        for (const FrameOut& f: anim.frames)
        {
            if (f.bEmpty)
            {
                ++a.emptyFrames;
            }
            else if (!a.bHasFoot)
            {
                a.footY0   = static_cast<double>(f.h - f.pivotY);
                a.bHasFoot = true;
            }
        }

        const AnimRecord& r = anim.rec;
        if (!r.bLoop && r.start > 0)
        {
            _state.anomalies.push_back({ "NonLoopStartNonZero", r.name, std::format("non-loop with Start={} (legacy Animator drew column End instead of the right column at the end); baked with correct columns", r.start), static_cast<double>(r.start) });
        }
        if (anim.cellW % 2 != 0)
        {
            _state.anomalies.push_back({ "OddCellWidth", r.name, std::format("cellW={} is odd; cellW/2 rounds down, mirroring Left/Right may be off by 1px", anim.cellW), static_cast<double>(anim.cellW) });
        }
        if (a.emptyFrames > 0)
        {
            _state.anomalies.push_back({ "EmptyFrame", r.name, std::format("{} of {} frames are fully transparent", a.emptyFrames, anim.frames.size()), static_cast<double>(a.emptyFrames) });
        }
        if (r.bLoop && r.optionIndex >= static_cast<int>(anim.frames.size()))
        {
            _state.anomalies.push_back({ "LoopStartOutOfRange", r.name, std::format("OptionIndex={} >= frame count {}", r.optionIndex, anim.frames.size()), static_cast<double>(r.optionIndex) });
        }
        if (!r.bLoop && r.optionIndex > 0 && r.optionIndex - r.start >= static_cast<int>(anim.frames.size()))
        {
            _state.anomalies.push_back({ "CancelFrameOutOfRange", r.name, std::format("OptionIndex={} Start={} beyond frame count {}", r.optionIndex, r.start, anim.frames.size()), static_cast<double>(r.optionIndex) });
        }
    }
    CheckLeftRightTwins_(_state, _state.anomalies);

    std::stable_sort(_state.anomalies.begin(), _state.anomalies.end(), [](const Anomaly& _a, const Anomaly& _b) {
        if (_a.type != _b.type)
        {
            return _a.type < _b.type;
        }
        return _a.value > _b.value;
    });

    CountMap anomalyCounts;
    for (const Anomaly& a: _state.anomalies)
    {
        ++anomalyCounts[a.type];
    }

    {
        Vector<size_t> byDrift;
        for (size_t i = 0; i < _state.anims.size(); ++i)
        {
            if (_state.anims[i].bValid)
            {
                byDrift.push_back(i);
            }
        }
        std::stable_sort(byDrift.begin(), byDrift.end(), [&](const size_t _a, const size_t _b) { return analysis[_a].drift.max > analysis[_b].drift.max; });

        JsonWriter w;
        w.BeginObject();
        w.Field("version", 1);
        w.Field("animationCount", static_cast<uint64_t>(byDrift.size()));

        w.Key("anomalyCounts");
        w.BeginObject();
        for (const CountMap::value_type& entry: anomalyCounts)
        {
            w.Field(entry.first, static_cast<uint64_t>(entry.second));
        }
        w.EndObject();

        w.Key("topDrift");
        w.BeginArray();
        for (size_t i = 0; i < std::min<size_t>(byDrift.size(), 40); ++i)
        {
            const AnimOut&     anim = _state.anims[byDrift[i]];
            const DriftResult& d    = analysis[byDrift[i]].drift;
            w.BeginObject(true);
            w.Field("name", anim.rec.name);
            w.Field("anchorDriftMax", d.max);
            w.Field("anchorDriftX", d.x);
            w.Field("anchorDriftY", d.y);
            w.EndObject();
        }
        w.EndArray();

        w.Key("animations");
        w.BeginObject();
        for (size_t i = 0; i < _state.anims.size(); ++i)
        {
            const AnimOut& anim = _state.anims[i];
            if (!anim.bValid)
            {
                continue;
            }
            const AnimAnalysis& a = analysis[i];
            w.Key(anim.rec.name);
            w.BeginObject(true);
            w.Field("category", CategoryKey(anim.category));
            w.Field("frames", static_cast<uint64_t>(anim.frames.size()));
            w.Field("emptyFrames", a.emptyFrames);
            w.Field("cellW", anim.cellW);
            w.Field("cellH", anim.cellH);
            w.Field("offsetX", anim.rec.offsetX);
            w.Field("offsetY", anim.rec.offsetY);
            w.Field("loop", anim.rec.bLoop);
            w.Field("anchorDriftMax", a.drift.max);
            w.Field("anchorDriftX", a.drift.x);
            w.Field("anchorDriftY", a.drift.y);
            w.Field("footY0", a.footY0);
            w.EndObject();
        }
        w.EndObject();

        w.Key("anomalies");
        w.BeginArray();
        for (const Anomaly& a: _state.anomalies)
        {
            w.BeginObject(true);
            w.Field("type", a.type);
            w.Field("name", a.animation);
            w.Field("detail", a.detail);
            w.Field("value", a.value);
            w.EndObject();
        }
        w.EndArray();

        w.EndObject();
        if (slug::Result<void> r = WriteText(reportDir / "pivot_report.json", w.Str()); !r)
        {
            return r;
        }
    }

    {
        Vector<SheetPlan> plans;
        for (const eAnimCategory category: kAllCategories)
        {
            String legend;
            PlanCategory_(_state, category, plans, legend);
            if (!legend.empty())
            {
                if (slug::Result<void> r = WriteText(reportDir / (String("contact_") + CategoryKey(category) + ".txt"), "index\tanimation\tcategory\tinfo\n" + legend); !r)
                {
                    return r;
                }
            }
        }

        for (const fs::directory_entry& entry: fs::directory_iterator(reportDir, ec))
        {
            const String name = entry.path().filename().string();
            if (name.starts_with("contact_") && entry.path().extension() == ".png")
            {
                fs::remove(entry.path(), ec);
            }
        }

        ContactSheetWriter writer { plans, _state, reportDir };
        ParallelFor(plans.size(), _options.threads, writer, &ContactSheetWriter::Write);
        if (writer.IsFailed())
        {
            Log("WARN: failed to write a contact sheet");
        }
        Log("contact sheets: {} file(s) in {}", plans.size(), reportDir.string());
    }

    {
        SummaryWriter report;

        size_t                                      validAnims      = 0;
        size_t                                      skippedAnims    = 0;
        size_t                                      validSprites    = 0;
        size_t                                      atlasSprites    = 0;
        size_t                                      fileSprites     = 0;
        size_t                                      implicitSprites = 0;
        size_t                                      skippedSprites  = 0;
        std::map<String, std::pair<size_t, size_t>> perCategory;
        for (const AnimOut& a: _state.anims)
        {
            if (a.bValid)
            {
                ++validAnims;
                std::pair<size_t, size_t>& entry = perCategory[CategoryKey(a.category)];
                ++entry.first;
                entry.second += a.frames.size();
            }
            else
            {
                ++skippedAnims;
            }
        }
        for (const SpriteOut& sp: _state.sprites)
        {
            if (!sp.bValid)
            {
                ++skippedSprites;
                continue;
            }
            ++validSprites;
            (sp.bFile ? fileSprites : atlasSprites) += 1;
            implicitSprites += sp.bImplicit ? 1 : 0;
        }
        size_t   texOk          = 0;
        size_t   texFailed      = 0;
        size_t   texStandalone  = 0;
        uint64_t partial        = 0;
        uint64_t nonPremult     = 0;
        uint64_t hidden         = 0;
        uint64_t magenta        = 0;
        size_t   texWithMagenta = 0;
        size_t   texNonPremult  = 0;
        for (const TextureStat& t: _state.textures)
        {
            (t.bOk ? texOk : texFailed) += 1;
            texStandalone += t.bStandalone ? 1 : 0;
            partial += t.partialAlpha;
            nonPremult += t.nonPremult;
            hidden += t.hiddenColor;
            magenta += t.opaqueMagenta;
            texWithMagenta += t.opaqueMagenta > 0 ? 1 : 0;
            texNonPremult += t.nonPremult > 0 ? 1 : 0;
        }
        size_t emptyFrames = 0;
        for (const AnimAnalysis& a: analysis)
        {
            emptyFrames += static_cast<size_t>(a.emptyFrames);
        }

        const double capacity = static_cast<double>(_state.pageCount) * _options.pageSize * _options.pageSize;

        report.Line("SlugBaker summary");
        report.Line("=================");
        report.Line(std::format("legacy root     : {}", _options.legacyRoot.string()));
        report.Line(std::format("output root     : {}", _options.outRoot.string()));
        report.Line(std::format("only            : {}", _options.only.empty() ? String("(all)") : JoinOnly_(_options.only)));
        report.Line(std::format("elapsed         : {:.1f} s", _state.elapsedSec));
        report.Line("");
        report.Line(std::format("textures defined: {}  (loaded {}, failed {}, standalone PNG {})", _state.textureCount, texOk, texFailed, texStandalone));
        report.Line(std::format("animations      : {} baked, {} skipped, {} frames ({} empty)", validAnims, skippedAnims, _state.totalFrames, emptyFrames));
        for (const std::pair<const String, std::pair<size_t, size_t>>& entry: perCategory)
        {
            report.Line(std::format("    {:<13}: {} animations, {} frames", entry.first, entry.second.first, entry.second.second));
        }
        report.Line(std::format("sprites         : {} baked ({} atlas, {} standalone file, {} implicit from unreferenced textures), {} skipped", validSprites, atlasSprites, fileSprites, implicitSprites, skippedSprites));
        report.Line(std::format("atlas           : {} page(s) of {}x{}, {} unique images, {} px used ({:.1f}% fill), dedupe saved {} frame refs", _state.pageCount, _options.pageSize, _options.pageSize, _state.uniques.size(), _state.uniquePixels, capacity > 0 ? 100.0 * static_cast<double>(_state.uniquePixels) / capacity : 0.0, _state.totalFrames + atlasSprites > _state.uniques.size() ? _state.totalFrames + atlasSprites - _state.uniques.size() : 0));
        {
            size_t ok = 0;
            for (const SoundOut& snd: _state.sounds)
            {
                ok += snd.bOk ? 1 : 0;
            }
            report.Line(std::format("sounds          : {} copied of {}", ok, _state.sounds.size()));
        }
        for (const GroundOut& g: _state.grounds)
        {
            if (g.bOk)
            {
                report.Line(std::format("ground {:<9}: {}x{} solid={} ({:.2f}%), unclassified={}, bounds x={} y={} w={} h={}  (B>={} R<={} G<={} A>={})", g.key, g.w, g.h, g.stats.solidCount, 100.0 * static_cast<double>(g.stats.solidCount) / static_cast<double>(std::max<uint64_t>(g.stats.totalCount, 1)), g.stats.unclassifiedCount, g.stats.solidBounds.x, g.stats.solidBounds.y, g.stats.solidBounds.w, g.stats.solidBounds.h, _options.ground.blueMin, _options.ground.redMax, _options.ground.greenMax, _options.ground.alphaMin));
            }
            else
            {
                report.Line(std::format("ground {:<9}: FAILED ({})", g.key, g.error));
            }
        }
        report.Line("");
        report.Line("alpha textures (UseTransparent=2) pixel statistics");
        report.Line(std::format("    partial-alpha pixels            : {}", partial));
        report.Line(std::format("    partial alpha with max(rgb) > a : {} in {} textures (> 0 means straight, not premultiplied, alpha)", nonPremult, texNonPremult));
        report.Line(std::format("    alpha==0 but rgb!=0 (cleared)   : {}", hidden));
        report.Line(std::format("    opaque magenta (255,0,255,255)  : {} px in {} textures", magenta, texWithMagenta));

        report.Line("");
        report.Line("anomalies by type");
        for (const CountMap::value_type& entry: anomalyCounts)
        {
            report.Line(std::format("    {:<28}: {}", entry.first, entry.second));
        }

        AppendTopAnomalies_(report, _state, "LeftRightOffsetMismatch", 15);
        AppendTopAnomalies_(report, _state, "LeftRightFootMismatch", 15);
        AppendTopAnomalies_(report, _state, "NonLoopStartNonZero", 20);
        AppendTopAnomalies_(report, _state, "MissingTexture", 20);
        AppendTopAnomalies_(report, _state, "MissingFile", 20);
        AppendTopAnomalies_(report, _state, "FrameOutOfBounds", 15);
        AppendTopAnomalies_(report, _state, "EmptyFrame", 15);
        AppendTopAnomalies_(report, _state, "OpaqueMagentaInFrame", 20);

        {
            Vector<size_t> byDrift;
            for (size_t i = 0; i < _state.anims.size(); ++i)
            {
                if (_state.anims[i].bValid)
                {
                    byDrift.push_back(i);
                }
            }
            std::stable_sort(byDrift.begin(), byDrift.end(), [&](const size_t _a, const size_t _b) { return analysis[_a].drift.max > analysis[_b].drift.max; });
            report.Line("");
            report.Line("top anchor drift (px, frame-to-frame bottom-center)");
            for (size_t i = 0; i < std::min<size_t>(byDrift.size(), 15); ++i)
            {
                const DriftResult& d = analysis[byDrift[i]].drift;
                report.Line(std::format("    {:<48} max={} x={} y={}", _state.anims[byDrift[i]].rec.name, Fmt1_(d.max), Fmt1_(d.x), Fmt1_(d.y)));
            }
        }

        if (!_state.parseWarnings.empty())
        {
            report.Line("");
            report.Line(std::format("parser warnings: {}", _state.parseWarnings.size()));
            for (size_t i = 0; i < std::min<size_t>(_state.parseWarnings.size(), 30); ++i)
            {
                report.Line("    " + _state.parseWarnings[i]);
            }
        }

        if (slug::Result<void> r = WriteText(reportDir / "bake_summary.txt", report.GetText()); !r)
        {
            return r;
        }
        std::fputs(report.GetText().c_str(), stdout);
        std::fflush(stdout);
    }

    return slug::Error { slug::kOK };
}

}   // namespace slug::baker
