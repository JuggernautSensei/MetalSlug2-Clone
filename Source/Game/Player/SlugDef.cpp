#include "Game/Player/SlugDef.h"

#include <cmath>
#include <format>
#include <optional>

#include "Base/SlugError.h"
#include "Base/Json.h"
#include "Base/Math.h"
#include "Base/Types.h"

namespace
{
// 수치 목록. 읽기/쓰기 방문자가 같은 목록을 쓰므로 키가 어긋나지 않는다.
template<typename V, typename T>
void Visit_(
    V& _v,
    T& _t)
{
    _v.Begin("body");
    _v.Field("standSize", _t.body.standSize);
    _v.Field("crouchSize", _t.body.crouchSize);
    _v.Field("hurtStand", _t.body.hurtStand);
    _v.Field("hurtCrouch", _t.body.hurtCrouch);
    _v.Field("footHalfWidth", _t.body.footHalfWidth);
    _v.End();

    _v.Begin("move");
    _v.Field("runSpeed", _t.move.runSpeed);
    _v.Field("crouchSpeed", _t.move.crouchSpeed);
    _v.Field("airSpeed", _t.move.airSpeed);
    _v.Field("accelTicks", _t.move.accelTicks);
    _v.Field("brakeTicks", _t.move.brakeTicks);
    _v.Field("jumpSpeed", _t.move.jumpSpeed);
    _v.Field("gravity", _t.move.gravity);
    _v.Field("terminalVelocity", _t.move.terminalVelocity);
    _v.Field("coyoteTicks", _t.move.coyoteTicks);
    _v.Field("jumpBufferTicks", _t.move.jumpBufferTicks);
    _v.Field("crouchStepTicks", _t.move.crouchStepTicks);
    _v.Field("landTicks", _t.move.landTicks);
    _v.Field("maxStepUp", _t.move.maxStepUp);
    _v.Field("maxSnapDown", _t.move.maxSnapDown);
    _v.End();

    _v.Begin("vulcan");
    _v.Field("mountStand", _t.vulcan.mountStand);
    _v.Field("mountCrouch", _t.vulcan.mountCrouch);
    _v.Field("barrelLength", _t.vulcan.barrelLength);
    _v.Field("sweepDegPerSec", _t.vulcan.sweepDegPerSec);
    _v.Field("fireIntervalTicks", _t.vulcan.fireIntervalTicks);
    _v.Field("bulletSpeed", _t.vulcan.bulletSpeed);
    _v.Field("bulletRange", _t.vulcan.bulletRange);
    _v.Field("damage", _t.vulcan.damage);
    _v.Field("bulletSize", _t.vulcan.bulletSize);
    _v.Field("shotFlashTicks", _t.vulcan.shotFlashTicks);
    _v.Field("maxAlive", _t.vulcan.maxAlive);
    _v.End();

    _v.Begin("cannon");
    _v.Field("ammoStart", _t.cannon.ammoStart);
    _v.Field("ammoMax", _t.cannon.ammoMax);
    _v.Field("cooldownTicks", _t.cannon.cooldownTicks);
    _v.Field("speed", _t.cannon.speed);
    _v.Field("gravity", _t.cannon.gravity);
    _v.Field("riseSpeed", _t.cannon.riseSpeed);
    _v.Field("upBoost", _t.cannon.upBoost);
    _v.Field("hitSize", _t.cannon.hitSize);
    _v.Field("damage", _t.cannon.damage);
    _v.Field("blastSize", _t.cannon.blastSize);
    _v.Field("blastUp", _t.cannon.blastUp);
    _v.Field("blastTicks", _t.cannon.blastTicks);
    _v.Field("muzzleStand", _t.cannon.muzzleStand);
    _v.Field("muzzleCrouch", _t.cannon.muzzleCrouch);
    _v.Field("muzzleFlashTicks", _t.cannon.muzzleFlashTicks);
    _v.Field("pickupAmmo", _t.cannon.pickupAmmo);
    _v.End();

    _v.Begin("damage");
    _v.Field("maxHp", _t.damage.maxHp);
    _v.Field("bulletDamage", _t.damage.bulletDamage);
    _v.Field("meleeDamage", _t.damage.meleeDamage);
    _v.Field("blastDamage", _t.damage.blastDamage);
    _v.Field("hazardDamage", _t.damage.hazardDamage);
    _v.Field("hitInvincibleTicks", _t.damage.hitInvincibleTicks);
    _v.Field("smokeRatio", _t.damage.smokeRatio);
    _v.Field("fireRatio", _t.damage.fireRatio);
    _v.Field("ejectInvincibleTicks", _t.damage.ejectInvincibleTicks);
    _v.Field("explodeDelayTicks", _t.damage.explodeDelayTicks);
    _v.Field("selfDestructTicks", _t.damage.selfDestructTicks);
    _v.Field("explosionBlast", _t.damage.explosionBlast);
    _v.Field("explosionDamage", _t.damage.explosionDamage);
    _v.Field("explosionBlastTicks", _t.damage.explosionBlastTicks);
    _v.End();

    _v.Begin("board");
    _v.Field("boardBoxSize", _t.board.boardBoxSize);
    _v.Field("boardTicks", _t.board.boardTicks);
    _v.Field("exitCooldownTicks", _t.board.exitCooldownTicks);
    _v.Field("exitJumpSpeed", _t.board.exitJumpSpeed);
    _v.Field("exitSpeedX", _t.board.exitSpeedX);
    _v.Field("exitTicks", _t.board.exitTicks);
    _v.Field("arrowGapPx", _t.board.arrowGapPx);
    _v.End();

    _v.Begin("crush");
    _v.Field("minSpeed", _t.crush.minSpeed);
    _v.Field("damage", _t.crush.damage);
    _v.Field("rehitTicks", _t.crush.rehitTicks);
    _v.Field("frontReach", _t.crush.frontReach);
    _v.End();
}
}   // namespace

class SlugDefReadVisitor
{
public:
    explicit SlugDefReadVisitor(
        const JsonReader& _root)
    {
        m_stack.push_back(_root.IsObject() ? std::optional<JsonReader> { _root } : std::nullopt);
        m_bError = !_root.IsObject();
    }

    void Begin(
        const StringView _key)
    {
        std::optional<JsonReader> child;
        if (const std::optional<JsonReader>& top = m_stack.back(); top && top->HasField(_key))
        {
            if (const Result<JsonReader> found = top->FindField(_key); found && found->IsObject())
            {
                child = found.GetValue();
            }
            else
            {
                m_bError = true;
            }
        }
        m_stack.push_back(child);
    }

    void End()
    {
        m_stack.pop_back();
    }

    void Field(
        const StringView _key,
        float&           _value)
    {
        double v = 0.0;
        if (ReadNumber_(_key, v))
        {
            _value = static_cast<float>(v);
        }
    }

    void Field(
        const StringView _key,
        int&             _value)
    {
        double v = 0.0;
        if (ReadNumber_(_key, v))
        {
            _value = static_cast<int>(std::llround(v));
        }
    }

    void Field(
        const StringView _key,
        Vec2&            _value)
    {
        const std::optional<JsonReader>& top = m_stack.back();
        if (!top || !top->HasField(_key))
        {
            return;
        }
        const Result<JsonReader> found = top->FindField(_key);
        if (!found || !found->IsArray() || found->GetSize() != 2)
        {
            m_bError = true;
            return;
        }

        ARRAY<float, 2> values = {};
        int             index  = 0;
        for (const JsonReader item: found->GetArray())
        {
            const Result<double> number = item.Read<double>();
            if (!number)
            {
                m_bError = true;
                return;
            }
            values[index++] = static_cast<float>(number.GetValue());
        }
        _value = { values[0], values[1] };
    }

    [[nodiscard]] bool HasError() const { return m_bError; }

private:
    bool ReadNumber_(
        const StringView _key,
        double&          _out)
    {
        const std::optional<JsonReader>& top = m_stack.back();
        if (!top || !top->HasField(_key))
        {
            return false;
        }
        const Result<double> value = top->ReadField<double>(_key);
        if (!value)
        {
            m_bError = true;
            return false;
        }
        _out = value.GetValue();
        return true;
    }

    Vector<std::optional<JsonReader>> m_stack  = {};
    bool                              m_bError = false;
};

// 중첩 객체를 들여쓴 JSON 으로 쓴다
class JsonWriter
{
public:
    JsonWriter()
    {
        m_text  = "{\n";
        m_depth = 1;
        m_bFirst.push_back(true);
    }

    void Begin(
        const StringView _key)
    {
        Comma_();
        Indent_();
        m_text += std::format("\"{}\": {{\n", String { _key });
        ++m_depth;
        m_bFirst.push_back(true);
    }

    void End()
    {
        m_bFirst.pop_back();
        --m_depth;
        m_text += '\n';
        Indent_();
        m_text += '}';
    }

    void Field(
        const StringView _key,
        const float      _value)
    {
        Line_(_key, FormatFloat_(_value));
    }

    void Field(
        const StringView _key,
        const int        _value)
    {
        Line_(_key, std::to_string(_value));
    }

    void Field(
        const StringView _key,
        const Vec2       _value)
    {
        Line_(_key, std::format("[{}, {}]", FormatFloat_(_value.x), FormatFloat_(_value.y)));
    }

    [[nodiscard]] String Finish()
    {
        m_text += "\n}\n";
        return m_text;
    }

private:
    [[nodiscard]] static String FormatFloat_(
        const float _value)
    {
        String text = std::format("{:.4f}", _value);
        while (text.size() > 1 && text.back() == '0')
        {
            text.pop_back();
        }
        if (!text.empty() && text.back() == '.')
        {
            text += '0';
        }
        return text;
    }

    void Comma_()
    {
        if (!m_bFirst.back())
        {
            m_text += ",\n";
        }
        m_bFirst.back() = false;
    }

    void Indent_()
    {
        for (int i = 0; i < m_depth; ++i)
        {
            m_text += "    ";
        }
    }

    void Line_(
        const StringView _key,
        const String&    _value)
    {
        Comma_();
        Indent_();
        m_text += std::format("\"{}\": {}", String { _key }, _value);
    }

    String       m_text   = {};
    int          m_depth  = 0;
    Vector<bool> m_bFirst = {};
};

namespace
{
[[nodiscard]] Result<SlugTuning> ParseRoot_(
    const JsonReader& _root)
{
    SlugTuning         tuning;
    SlugDefReadVisitor visitor { _root };
    Visit_(visitor, tuning);
    if (visitor.HasError())
    {
        return Error { eError::Parse };
    }
    return tuning;
}
}   // namespace

Result<SlugTuning> ParseSlugTuning(
    const StringView _json)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json, eJsonLoadOption::AllowComments);
    if (!doc)
    {
        return Error { eError::Parse };
    }
    return ParseRoot_(doc->GetReader());
}

Result<SlugTuning> LoadSlugTuning(
    const FilePath& _path)
{
    Result<JsonDeserializer> doc = JsonDeserializer::LoadFromFile(_path, eJsonLoadOption::AllowComments);
    if (!doc)
    {
        return Error { eError::FileOpen };
    }
    return ParseRoot_(doc->GetReader());
}

String DumpSlugTuningJson(
    const SlugTuning& _tuning)
{
    SlugTuning copy = _tuning;
    JsonWriter writer;
    Visit_(writer, copy);
    return writer.Finish();
}

FilePath GetDefaultSlugTuningPath(
    const FilePath& _assetRoot)
{
    return _assetRoot / "Defs" / "Slug" / "Slug.json";
}
