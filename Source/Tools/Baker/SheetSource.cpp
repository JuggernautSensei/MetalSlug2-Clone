#include "Tools/Baker/SheetSource.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>

#include <yyjson.h>

#include "Core/AssetPath.h"
#include "Tools/Baker/BakerUtil.h"
#include "Tools/Baker/PngWriter.h"

namespace slug::baker
{

namespace fs = std::filesystem;

namespace
{
    using JsonDocPtr = std::unique_ptr<yyjson_doc, decltype(&yyjson_doc_free)>;

    [[nodiscard]] bool ReadInt_(
        yyjson_val* _pValue,
        int&        _out)
    {
        if (yyjson_is_sint(_pValue)) { _out = static_cast<int>(yyjson_get_sint(_pValue)); return true; }
        if (yyjson_is_uint(_pValue)) { _out = static_cast<int>(yyjson_get_uint(_pValue)); return true; }
        if (yyjson_is_real(_pValue)) { _out = static_cast<int>(yyjson_get_real(_pValue)); return true; }
        return false;
    }

    [[nodiscard]] bool ReadRect_(
        yyjson_val* _pValue,
        IRect&      _out)
    {
        if (_pValue == nullptr || !yyjson_is_arr(_pValue) || yyjson_arr_size(_pValue) != 4)
        {
            return false;
        }
        int v[4] = {};
        for (size_t i = 0; i < 4; ++i)
        {
            if (!ReadInt_(yyjson_arr_get(_pValue, i), v[i]))
            {
                return false;
            }
        }
        _out = { v[0], v[1], v[2], v[3] };
        return true;
    }

    // 값이 없으면 기본값 유지(true), 있는데 형식이 틀리면 false.
    [[nodiscard]] bool OptInt_(
        yyjson_val* _pObj,
        const char* _key,
        int&        _out)
    {
        yyjson_val* v = yyjson_obj_get(_pObj, _key);
        return v == nullptr || ReadInt_(v, _out);
    }

    [[nodiscard]] bool OptBool_(
        yyjson_val* _pObj,
        const char* _key,
        bool&       _out)
    {
        yyjson_val* v = yyjson_obj_get(_pObj, _key);
        if (v == nullptr) { return true; }
        if (!yyjson_is_bool(v)) { return false; }
        _out = yyjson_get_bool(v);
        return true;
    }

    [[nodiscard]] bool OptReal_(
        yyjson_val* _pObj,
        const char* _key,
        double&     _out)
    {
        yyjson_val* v = yyjson_obj_get(_pObj, _key);
        if (v == nullptr) { return true; }
        if (yyjson_is_real(v)) { _out = yyjson_get_real(v); return true; }
        int i = 0;
        if (ReadInt_(v, i)) { _out = static_cast<double>(i); return true; }
        return false;
    }

    [[nodiscard]] bool ReadName_(
        yyjson_val*  _pObj,
        const char*  _key,
        std::string& _out)
    {
        yyjson_val* v = yyjson_obj_get(_pObj, _key);
        if (v == nullptr || !yyjson_is_str(v)) { return false; }
        _out = yyjson_get_str(v);
        return true;
    }

    [[nodiscard]] bool ParseClip_(
        yyjson_val*     _pClip,
        SheetClipSpec&  _out,
        std::string&    _error)
    {
        if (!ReadName_(_pClip, "name", _out.name) || _out.name.empty())
        {
            _error = "clip without name";
            return false;
        }

        if (yyjson_val* region = yyjson_obj_get(_pClip, "region"))
        {
            if (!ReadRect_(region, _out.region)) { _error = "clip '" + _out.name + "': region must be [x,y,w,h]"; return false; }
            _out.bHasRegion = true;
        }

        if (yyjson_val* frames = yyjson_obj_get(_pClip, "frames"))
        {
            if (!yyjson_is_arr(frames)) { _error = "clip '" + _out.name + "': frames must be an array"; return false; }
            size_t idx = 0;
            size_t max = 0;
            yyjson_val* f = nullptr;
            yyjson_arr_foreach(frames, idx, max, f)
            {
                IRect r;
                if (!ReadRect_(f, r)) { _error = "clip '" + _out.name + "': frame must be [x,y,w,h]"; return false; }
                _out.frames.push_back(r);
            }
        }

        if (yyjson_val* autoV = yyjson_obj_get(_pClip, "auto"))
        {
            _out.bAuto = true;
            if (yyjson_is_obj(autoV))
            {
                bool bRows = _out.autoOpts.bRows;
                if (!OptBool_(autoV, "rows", bRows) || !OptInt_(autoV, "mergeGap", _out.autoOpts.mergeGap) || !OptInt_(autoV, "minPixels", _out.autoOpts.minPixels))
                {
                    _error = "clip '" + _out.name + "': bad auto fields";
                    return false;
                }
                _out.autoOpts.bRows = bRows;
            }
            else if (!yyjson_is_bool(autoV) || !yyjson_get_bool(autoV))
            {
                _out.bAuto = false;
            }
        }

        if (_out.frames.empty() && !_out.bAuto)
        {
            _error = "clip '" + _out.name + "': needs 'frames' or 'auto'";
            return false;
        }

        std::string pivot;
        if (yyjson_obj_get(_pClip, "pivot") != nullptr)
        {
            if (!ReadName_(_pClip, "pivot", pivot) || (pivot != "foot" && pivot != "center"))
            {
                _error = "clip '" + _out.name + "': pivot must be \"foot\" or \"center\"";
                return false;
            }
            _out.pivot = (pivot == "center") ? eSheetPivot::Center : eSheetPivot::Foot;
        }

        if (!OptBool_(_pClip, "loop", _out.bLoop) || !OptReal_(_pClip, "fps", _out.fps) || !OptBool_(_pClip, "mirror", _out.bMirror)
            || !OptBool_(_pClip, "asSprite", _out.bAsSprite) || !OptBool_(_pClip, "replace", _out.bReplace))
        {
            _error = "clip '" + _out.name + "': bad loop/fps/mirror/asSprite/replace";
            return false;
        }
        if (!(_out.fps > 0.0))
        {
            _error = "clip '" + _out.name + "': fps must be > 0";
            return false;
        }
        return true;
    }

    [[nodiscard]] bool ParseSheet_(
        yyjson_val* _pSheet,
        SheetSpec&  _out,
        std::string& _error)
    {
        if (!ReadName_(_pSheet, "file", _out.file) || _out.file.empty())
        {
            _error = "sheet without file";
            return false;
        }

        if (yyjson_val* key = yyjson_obj_get(_pSheet, "key"))
        {
            std::string mode;
            if (!yyjson_is_obj(key) || (yyjson_obj_get(key, "mode") && !ReadName_(key, "mode", mode)))
            {
                _error = _out.file + ": bad key";
                return false;
            }
            if (mode == "color") { _out.key.mode = eSheetKeyMode::Color; }
            else if (mode == "alpha") { _out.key.mode = eSheetKeyMode::Alpha; }
            else if (mode.empty() || mode == "magenta") { _out.key.mode = eSheetKeyMode::Magenta; }
            else { _error = _out.file + ": key.mode must be color|alpha|magenta"; return false; }

            if (yyjson_val* colors = yyjson_obj_get(key, "colors"))
            {
                if (!yyjson_is_arr(colors)) { _error = _out.file + ": key.colors must be an array"; return false; }
                size_t idx = 0;
                size_t max = 0;
                yyjson_val* c = nullptr;
                yyjson_arr_foreach(colors, idx, max, c)
                {
                    if (!yyjson_is_arr(c) || yyjson_arr_size(c) != 3) { _error = _out.file + ": key.colors entries must be [r,g,b]"; return false; }
                    std::array<uint8_t, 3> rgbArr = {};
                    for (size_t k = 0; k < 3; ++k)
                    {
                        int v = 0;
                        if (!ReadInt_(yyjson_arr_get(c, k), v) || v < 0 || v > 255) { _error = _out.file + ": key.colors value out of range"; return false; }
                        rgbArr[k] = static_cast<uint8_t>(v);
                    }
                    _out.key.colors.push_back(rgbArr);
                }
            }
            if (!OptInt_(key, "tol", _out.key.tol))
            {
                _error = _out.file + ": key.tol must be an integer";
                return false;
            }
        }
        if (_out.key.mode == eSheetKeyMode::Color && _out.key.colors.empty())
        {
            _error = _out.file + ": key.mode color needs key.colors";
            return false;
        }

        if (yyjson_val* exclude = yyjson_obj_get(_pSheet, "exclude"))
        {
            if (!yyjson_is_arr(exclude)) { _error = _out.file + ": exclude must be an array"; return false; }
            size_t idx = 0;
            size_t max = 0;
            yyjson_val* e = nullptr;
            yyjson_arr_foreach(exclude, idx, max, e)
            {
                IRect r;
                if (!ReadRect_(e, r)) { _error = _out.file + ": exclude entry must be [x,y,w,h]"; return false; }
                _out.exclude.push_back(r);
            }
        }

        if (yyjson_val* clips = yyjson_obj_get(_pSheet, "clips"))
        {
            if (!yyjson_is_arr(clips)) { _error = _out.file + ": clips must be an array"; return false; }
            size_t idx = 0;
            size_t max = 0;
            yyjson_val* c = nullptr;
            yyjson_arr_foreach(clips, idx, max, c)
            {
                SheetClipSpec clip;
                if (!ParseClip_(c, clip, _error))
                {
                    _error = _out.file + ": " + _error;
                    return false;
                }
                _out.clips.push_back(std::move(clip));
            }
        }
        return true;
    }

    // 클립의 시트 좌표계 사각형: 명시 frames 가 있으면 그것, 아니면 auto 슬라이스.
    [[nodiscard]] std::vector<IRect> ClipRects_(
        const Image&        _image,
        const SheetSpec&    _spec,
        const SheetClipSpec& _clip)
    {
        if (!_clip.frames.empty())
        {
            return _clip.frames;
        }
        const IRect region = _clip.bHasRegion ? _clip.region : IRect { 0, 0, _image.width, _image.height };
        return SliceAuto(_image, region, _clip.autoOpts, _spec.exclude);
    }

    struct BuiltFrames
    {
        std::vector<FrameOut> frames;
        std::vector<Image>    trimmed;
        int                   cellW = 0;
        int                   cellH = 0;
    };

    // 사각형 -> 알파 기준 트림 -> 피벗. bFlip 이면 좌우 반전 프레임(피벗 x 도 뒤집음).
    [[nodiscard]] BuiltFrames BuildFrames_(
        const Image&              _image,
        const std::vector<IRect>& _rects,
        const eSheetPivot         _pivot,
        const bool                _bFlip)
    {
        BuiltFrames out;
        for (const IRect& rect: _rects)
        {
            out.cellW = std::max(out.cellW, rect.w);
            out.cellH = std::max(out.cellH, rect.h);

            FrameOut    f;
            Image       crop   = CropImage(_image, rect);
            const IRect bounds = FindOpaqueBounds(crop, { 0, 0, crop.width, crop.height }, 0);
            if (bounds.IsEmpty())
            {
                f.bEmpty = true;
                out.frames.push_back(f);
                out.trimmed.emplace_back();
                continue;
            }

            Image trimmed = CropImage(crop, bounds);
            f.trimX       = bounds.x;
            f.trimY       = bounds.y;
            f.w           = bounds.w;
            f.h           = bounds.h;
            Pivot pivot   = ComputeSheetPivot(f.w, f.h, _pivot);
            if (_bFlip)
            {
                FlipHorizontal(trimmed);
                pivot.x = f.w - pivot.x;
            }
            f.pivotX = pivot.x;
            f.pivotY = pivot.y;
            out.frames.push_back(f);
            out.trimmed.push_back(std::move(trimmed));
        }
        return out;
    }

    [[nodiscard]] bool HasValidAnim_(
        const BakeState&   _state,
        const std::string& _name)
    {
        for (const AnimOut& a: _state.anims)
        {
            if (a.bValid && a.rec.name == _name) { return true; }
        }
        return false;
    }

    [[nodiscard]] bool HasValidSprite_(
        const BakeState&   _state,
        const std::string& _name)
    {
        for (const SpriteOut& s: _state.sprites)
        {
            if (s.bValid && s.rec.name == _name) { return true; }
        }
        return false;
    }

    void InvalidateAnim_(
        BakeState&         _state,
        const std::string& _name,
        const std::string& _reason)
    {
        for (AnimOut& a: _state.anims)
        {
            if (a.bValid && a.rec.name == _name)
            {
                a.bValid      = false;
                a.skipReason  = _reason;
            }
        }
    }

    void InvalidateSprite_(
        BakeState&         _state,
        const std::string& _name,
        const std::string& _reason)
    {
        for (SpriteOut& s: _state.sprites)
        {
            if (s.bValid && s.rec.name == _name)
            {
                s.bValid     = false;
                s.skipReason = _reason;
            }
        }
    }

    // ---- 미리보기 그리기 (3x5 숫자 글꼴, 소스 해상도 기준) ----
    constexpr uint8_t kDigitRows[10][5] = {
        { 0b111, 0b101, 0b101, 0b101, 0b111 },
        { 0b010, 0b110, 0b010, 0b010, 0b111 },
        { 0b111, 0b001, 0b111, 0b100, 0b111 },
        { 0b111, 0b001, 0b111, 0b001, 0b111 },
        { 0b101, 0b101, 0b111, 0b001, 0b001 },
        { 0b111, 0b100, 0b111, 0b001, 0b111 },
        { 0b111, 0b100, 0b111, 0b101, 0b111 },
        { 0b111, 0b001, 0b001, 0b001, 0b001 },
        { 0b111, 0b101, 0b111, 0b101, 0b111 },
        { 0b111, 0b101, 0b111, 0b001, 0b111 }
    };

    void PutPixel_(
        Image&                       _image,
        const int                    _x,
        const int                    _y,
        const std::array<uint8_t, 4>& _color)
    {
        if (_x < 0 || _y < 0 || _x >= _image.width || _y >= _image.height)
        {
            return;
        }
        uint8_t* p = _image.At(_x, _y);
        p[0] = _color[0];
        p[1] = _color[1];
        p[2] = _color[2];
        p[3] = _color[3];
    }

    void DrawOutline_(
        Image&                        _image,
        const IRect&                  _rect,
        const std::array<uint8_t, 4>& _color,
        const int                     _thickness)
    {
        for (int t = 0; t < _thickness; ++t)
        {
            for (int x = _rect.x - t; x < _rect.x + _rect.w + t; ++x)
            {
                PutPixel_(_image, x, _rect.y - t, _color);
                PutPixel_(_image, x, _rect.y + _rect.h - 1 + t, _color);
            }
            for (int y = _rect.y - t; y < _rect.y + _rect.h + t; ++y)
            {
                PutPixel_(_image, _rect.x - t, y, _color);
                PutPixel_(_image, _rect.x + _rect.w - 1 + t, y, _color);
            }
        }
    }

    // 사각형 왼쪽 위에 검은 바탕 + 흰 숫자 (2px 셀).
    void DrawLabel_(
        Image&       _image,
        const IRect& _rect,
        const int    _value,
        const std::array<uint8_t, 4>& _color)
    {
        const std::string text = std::to_string(_value);
        const int         boxW = static_cast<int>(text.size()) * 8 + 2;
        const int         boxH = 12;
        for (int y = 0; y < boxH; ++y)
        {
            for (int x = 0; x < boxW; ++x)
            {
                PutPixel_(_image, _rect.x + x, _rect.y - boxH + y, { 0, 0, 0, 255 });
            }
        }
        for (size_t i = 0; i < text.size(); ++i)
        {
            const uint8_t* rows = kDigitRows[text[i] - '0'];
            for (int ry = 0; ry < 5; ++ry)
            {
                for (int rx = 0; rx < 3; ++rx)
                {
                    if (rows[ry] & (1u << (2 - rx)))
                    {
                        for (int dy = 0; dy < 2; ++dy)
                        {
                            for (int dx = 0; dx < 2; ++dx)
                            {
                                PutPixel_(_image, _rect.x + 1 + static_cast<int>(i) * 8 + rx * 2 + dx, _rect.y - boxH + 1 + ry * 2 + dy, _color);
                            }
                        }
                    }
                }
            }
        }
    }

    [[nodiscard]] Image ScaleNearest_(
        const Image& _src,
        const int    _scale)
    {
        Image dst = Image::Make(_src.width * _scale, _src.height * _scale);
        for (int y = 0; y < dst.height; ++y)
        {
            for (int x = 0; x < dst.width; ++x)
            {
                std::memcpy(dst.At(x, y), _src.At(x / _scale, y / _scale), 4);
            }
        }
        return dst;
    }
}

slug::Result<void> LoadSheetConfig(
    const fs::path& _path,
    SheetConfig&    _out)
{
    slug::Result<std::string> text = ReadText(_path);
    if (!text)
    {
        Log("ERROR: cannot read {}", _path.string());
        return slug::Error { slug::eError::FileOpen };
    }

    // 메모장/PowerShell 이 붙이는 UTF-8 BOM 은 건너뛴다.
    const std::string& json   = *text;
    const char*        pData  = json.data();
    size_t             nBytes = json.size();
    if (nBytes >= 3 && static_cast<uint8_t>(pData[0]) == 0xEF && static_cast<uint8_t>(pData[1]) == 0xBB && static_cast<uint8_t>(pData[2]) == 0xBF)
    {
        pData += 3;
        nBytes -= 3;
    }
    JsonDocPtr doc(yyjson_read(pData, nBytes, 0), &yyjson_doc_free);
    yyjson_val*        root = doc ? yyjson_doc_get_root(doc.get()) : nullptr;
    if (root == nullptr || !yyjson_is_obj(root))
    {
        Log("ERROR: {} is not a JSON object", _path.string());
        return slug::Error { slug::eError::InvalidData };
    }

    yyjson_val* sheets = yyjson_obj_get(root, "sheets");
    if (sheets == nullptr || !yyjson_is_arr(sheets))
    {
        Log("ERROR: {}: 'sheets' array missing", _path.string());
        return slug::Error { slug::eError::InvalidData };
    }

    _out.path = _path;
    _out.sheets.clear();
    size_t idx = 0;
    size_t max = 0;
    yyjson_val* s = nullptr;
    yyjson_arr_foreach(sheets, idx, max, s)
    {
        SheetSpec spec;
        std::string error;
        if (!ParseSheet_(s, spec, error))
        {
            Log("ERROR: {}: {}", _path.string(), error);
            return slug::Error { slug::eError::InvalidData };
        }
        _out.sheets.push_back(std::move(spec));
    }
    return slug::Error { slug::kOK };
}

slug::Result<void> AppendSheetClips(
    const BakerOptions& _options,
    BakeState&          _state)
{
    if (_options.bNoSheets || !_options.Wants("sheet"))
    {
        return slug::Error { slug::kOK };
    }

    fs::path sheetsPath = _options.sheetsPath;
    if (sheetsPath.empty())
    {
        sheetsPath = slug::FindAssetRoot() / "Source" / "sheets.json";
        if (!fs::is_regular_file(sheetsPath))
        {
            return slug::Error { slug::kOK };
        }
    }
    else if (!fs::is_regular_file(sheetsPath))
    {
        Log("ERROR: sheets file not found: {}", sheetsPath.string());
        return slug::Error { slug::eError::NotFound };
    }

    SheetConfig config;
    if (slug::Result<void> r = LoadSheetConfig(sheetsPath, config); !r)
    {
        return r;
    }

    size_t clipCount = 0;
    for (const SheetSpec& spec: config.sheets)
    {
        const fs::path            imagePath = config.path.parent_path() / spec.file;
        slug::Result<std::string> bytes     = ReadText(imagePath);
        Image                     image;
        if (!bytes || !DecodeImageBytes(reinterpret_cast<const uint8_t*>((*bytes).data()), (*bytes).size(), image))
        {
            Log("WARN: sheet {} cannot be decoded", imagePath.string());
            _state.anomalies.push_back({ "SheetLoadFailed", spec.file, imagePath.string(), 0.0 });
            continue;
        }

        const uint64_t keyedPixels = ApplySheetKey(image, spec.key);
        Log("sheet {}: {}x{} keyed {} background px, {} clip(s)", spec.file, image.width, image.height, keyedPixels, spec.clips.size());

        for (const SheetClipSpec& clip: spec.clips)
        {
            const std::vector<IRect> rects = ClipRects_(image, spec, clip);
            if (rects.empty())
            {
                Log("WARN: sheet clip {}: no content found in {}", clip.name, spec.file);
                _state.anomalies.push_back({ "SheetClipEmpty", clip.name, spec.file, 0.0 });
                continue;
            }
            if (clip.bAsSprite && rects.size() != 1)
            {
                Log("WARN: sheet clip {}: asSprite needs exactly one frame, got {}", clip.name, rects.size());
                _state.anomalies.push_back({ "SheetSpriteFrameCount", clip.name, std::to_string(rects.size()) + " frames", 0.0 });
                continue;
            }

            struct Variant
            {
                std::string name;
                bool        bFlip = false;
            };
            std::vector<Variant> variants = { { clip.name, false } };
            if (clip.bMirror)
            {
                std::string mirrored;
                if (MirrorClipName(clip.name, mirrored))
                {
                    variants.push_back({ mirrored, true });
                }
                else
                {
                    Log("WARN: sheet clip {}: mirror needs a _Left/_Right token in the name", clip.name);
                    _state.anomalies.push_back({ "SheetMirrorSkipped", clip.name, "name has no _Left/_Right token", 0.0 });
                }
            }

            for (const Variant& v: variants)
            {
                const bool bExists = clip.bAsSprite ? HasValidSprite_(_state, v.name) : HasValidAnim_(_state, v.name);
                const eSheetNameAction action = ResolveSheetName(bExists, clip.bReplace);
                if (action == eSheetNameAction::Skip)
                {
                    Log("WARN: sheet clip {} ({}) skipped: name already exists (set replace:true to override)", v.name, spec.file);
                    _state.anomalies.push_back({ "SheetNameCollision", v.name, "already defined; sheet clip from " + spec.file + " skipped", 0.0 });
                    continue;
                }

                BuiltFrames built = BuildFrames_(image, rects, clip.pivot, v.bFlip);
                if (action == eSheetNameAction::Replace)
                {
                    const std::string reason = "replaced by sheet clip from " + spec.file;
                    if (clip.bAsSprite) { InvalidateSprite_(_state, v.name, reason); }
                    else { InvalidateAnim_(_state, v.name, reason); }
                    _state.anomalies.push_back({ "SheetReplaced", v.name, reason, 0.0 });
                }

                if (clip.bAsSprite)
                {
                    if (built.frames[0].bEmpty)
                    {
                        continue;
                    }
                    const FrameOut& f = built.frames[0];
                    SpriteOut       sprite;
                    sprite.rec.name      = v.name;
                    sprite.rec.texture   = spec.file;
                    sprite.rec.sourceFile = config.path.filename().string();
                    sprite.bValid        = true;
                    sprite.texW          = image.width;
                    sprite.texH          = image.height;
                    sprite.cellW         = f.w;
                    sprite.cellH         = f.h;
                    sprite.bCustomPivot  = true;
                    sprite.pivotX        = f.pivotX;
                    sprite.pivotY        = f.pivotY;
                    sprite.image         = std::move(built.trimmed[0]);
                    _state.sprites.push_back(std::move(sprite));
                    ++clipCount;
                    continue;
                }

                AnimOut out;
                out.category          = eAnimCategory::Sheet;
                out.bValid            = true;
                out.rec.name          = v.name;
                out.rec.texture       = spec.file;
                out.rec.sourceFile    = config.path.filename().string();
                out.rec.start         = 0;
                out.rec.end           = static_cast<int>(rects.size()) - 1;
                out.rec.width         = built.cellW;
                out.rec.height        = built.cellH;
                out.rec.frameDuration = 1.0 / clip.fps;
                out.rec.bLoop         = clip.bLoop;
                out.rec.optionIndex   = 0;
                out.cellW             = built.cellW;
                out.cellH             = built.cellH;
                out.frames            = std::move(built.frames);
                out.trimmed           = std::move(built.trimmed);
                _state.anims.push_back(std::move(out));
                ++clipCount;
            }
        }
    }

    Log("sheets: {} clip(s) added from {}", clipCount, config.sheets.size());
    return slug::Error { slug::kOK };
}

slug::Result<void> RunSheetPreview(const BakerOptions& _options)
{
    const fs::path defaultPath = slug::FindAssetRoot() / "Source" / "sheets.json";
    fs::path       sheetsPath  = _options.sheetsPath.empty() ? defaultPath : _options.sheetsPath;

    SheetConfig config;
    config.path = sheetsPath;
    if (fs::is_regular_file(sheetsPath))
    {
        if (slug::Result<void> r = LoadSheetConfig(sheetsPath, config); !r)
        {
            return r;
        }
    }

    const fs::path arg  = _options.sheetPreview;
    const std::string want = arg.filename().string();
    SheetSpec  fallback;
    const SheetSpec* pSpec = nullptr;
    for (const SheetSpec& spec: config.sheets)
    {
        if (fs::path(spec.file).filename().string() == want)
        {
            pSpec = &spec;
            break;
        }
    }
    SheetClipSpec autoClip;
    autoClip.name  = "auto";
    autoClip.bAuto = true;
    if (pSpec == nullptr)
    {
        fallback.file = want;
        fallback.clips.push_back(autoClip);
        pSpec = &fallback;
        Log("no sheets.json entry for {}: previewing a default magenta key + auto slice", want);
    }
    else if (pSpec->clips.empty())
    {
        fallback      = *pSpec;
        fallback.clips.push_back(autoClip);
        pSpec = &fallback;
        Log("sheets.json entry for {} has no clips: previewing the key + whole-sheet auto slice", want);
    }

    fs::path imagePath = arg;
    if (!fs::is_regular_file(imagePath))
    {
        imagePath = config.path.parent_path() / pSpec->file;
    }
    slug::Result<std::string> bytes = ReadText(imagePath);
    Image                     source;
    if (!bytes || !DecodeImageBytes(reinterpret_cast<const uint8_t*>((*bytes).data()), (*bytes).size(), source))
    {
        Log("ERROR: cannot decode sheet {}", imagePath.string());
        return slug::Error { slug::eError::NotFound };
    }

    Image keyed = source;
    ApplySheetKey(keyed, pSpec->key);

    // 배경(알파 0)은 어두운 회색으로, 나머지는 원본 색으로 보이게 한다.
    Image canvas = keyed;
    for (size_t i = 0; i < canvas.pixels.size(); i += 4)
    {
        if (canvas.pixels[i + 3] == 0)
        {
            canvas.pixels[i] = canvas.pixels[i + 1] = canvas.pixels[i + 2] = 40;
        }
        canvas.pixels[i + 3] = 255;
    }

    const int thickness = std::max(2, canvas.width / 1200);
    for (const IRect& ex: pSpec->exclude)
    {
        DrawOutline_(canvas, ex, { 255, 0, 0, 255 }, thickness);
    }

    static constexpr std::array<uint8_t, 4> kPalette[] = {
        { 255, 255, 0, 255 }, { 0, 255, 255, 255 }, { 255, 128, 0, 255 },
        { 128, 255, 128, 255 }, { 255, 128, 255, 255 }, { 128, 128, 255, 255 }
    };
    for (size_t ci = 0; ci < pSpec->clips.size(); ++ci)
    {
        const SheetClipSpec& clip  = pSpec->clips[ci];
        const auto           color = kPalette[ci % std::size(kPalette)];
        const std::vector<IRect> rects = ClipRects_(keyed, *pSpec, clip);
        Log("clip {} [{}]: {} rect(s)", clip.name, ci, rects.size());
        for (size_t i = 0; i < rects.size() && i < 200; ++i)
        {
            Log("  {} {},{} {}x{}", i, rects[i].x, rects[i].y, rects[i].w, rects[i].h);
            DrawOutline_(canvas, rects[i], color, thickness);
            DrawLabel_(canvas, rects[i], static_cast<int>(i), color);
        }
    }

    Image output = _options.sheetPreviewScale > 1 ? ScaleNearest_(canvas, _options.sheetPreviewScale) : canvas;
    const fs::path outDir = slug::FindAssetRoot().parent_path() / "Captures" / "SheetPreview";
    std::error_code ec;
    fs::create_directories(outDir, ec);
    std::vector<uint8_t> png;
    if (!EncodePng(output, png))
    {
        return slug::Error { slug::eError::InvalidData };
    }
    const fs::path outPath = outDir / (imagePath.stem().string() + ".png");
    if (slug::Result<void> r = WriteBytes(outPath, png.data(), png.size()); !r)
    {
        return r;
    }
    Log("sheet preview: {} ({}x{})", outPath.string(), output.width, output.height);
    return slug::Error { slug::kOK };
}
}
