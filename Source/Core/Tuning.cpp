#include "Core/Tuning.h"

#include <cmath>
#include <format>
#include <optional>
#include <string>

#include "Base/Error.h"
#include "Base/Json.h"
#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"

namespace slug
{

namespace
{
    template<typename V, typename W>
    void VisitWeapon_(
        V& _v,
        W& _w)
    {
        _v.Field("damage", _w.damage);
        _v.Field("fireIntervalTicks", _w.fireIntervalTicks);
        _v.Field("holdRepeatTicks", _w.holdRepeatTicks);
        _v.Field("ammo", _w.ammo);
        _v.Field("projectileSpeed", _w.projectileSpeed);
        _v.Field("maxAlive", _w.maxAlive);
        _v.Field("pierce", _w.bPierce);
        _v.Field("hitSize", _w.hitSize);
        _v.Field("range", _w.range);
        _v.Field("shotOverlayTicks", _w.shotOverlayTicks);
        _v.Field("blastSize", _w.blastSize);
        _v.Field("blastDamage", _w.blastDamage);
        _v.Field("blastTicks", _w.blastTicks);
    }

    template<typename V, typename S>
    void VisitMuzzleSet_(
        V& _v,
        S& _s)
    {
        _v.Field("forward", _s.forward);
        _v.Field("up", _s.up);
        _v.Field("diagUp", _s.diagUp);
        _v.Field("down", _s.down);
        _v.Field("diagDown", _s.diagDown);
    }

    template<typename V, typename T>
    void Visit_(
        V& _v,
        T& _t)
    {
        _v.Begin("move");
        _v.Field("runSpeed", _t.move.runSpeed);
        _v.Field("crouchSpeed", _t.move.crouchSpeed);
        _v.Field("airSpeed", _t.move.airSpeed);
        _v.Field("jumpSpeed", _t.move.jumpSpeed);
        _v.Field("gravity", _t.move.gravity);
        _v.Field("terminalVelocity", _t.move.terminalVelocity);
        _v.Field("coyoteTicks", _t.move.coyoteTicks);
        _v.Field("jumpBufferTicks", _t.move.jumpBufferTicks);
        _v.Field("shotBufferTicks", _t.move.shotBufferTicks);
        _v.Field("bombBufferTicks", _t.move.bombBufferTicks);
        _v.Field("maxStepUp", _t.move.maxStepUp);
        _v.Field("maxSnapDown", _t.move.maxSnapDown);
        _v.Field("jumpHeadroom", _t.move.jumpHeadroom);
        _v.End();

        _v.Begin("body");
        _v.Field("standSize", _t.body.standSize);
        _v.Field("crouchSize", _t.body.crouchSize);
        _v.Field("hurtStand", _t.body.hurtStand);
        _v.Field("hurtCrouch", _t.body.hurtCrouch);
        _v.Field("footHalfWidth", _t.body.footHalfWidth);
        _v.End();

        _v.Begin("muzzle");
        _v.Begin("stand");
        VisitMuzzleSet_(_v, _t.muzzle.stand);
        _v.End();
        _v.Begin("crouch");
        VisitMuzzleSet_(_v, _t.muzzle.crouch);
        _v.End();
        _v.Begin("air");
        VisitMuzzleSet_(_v, _t.muzzle.air);
        _v.End();
        _v.End();

        _v.Begin("weapons");
        _v.Begin("pistol");
        VisitWeapon_(_v, _t.weapons.pistol);
        _v.End();
        _v.Begin("hmg");
        VisitWeapon_(_v, _t.weapons.hmg);
        _v.End();
        _v.Begin("shotgun");
        VisitWeapon_(_v, _t.weapons.shotgun);
        _v.End();
        _v.Begin("rocket");
        VisitWeapon_(_v, _t.weapons.rocket);
        _v.End();
        _v.Begin("flame");
        VisitWeapon_(_v, _t.weapons.flame);
        _v.End();
        _v.Field("hmgPickupAmmo", _t.weapons.hmgPickupAmmo);
        _v.Field("shotgunPickupAmmo", _t.weapons.shotgunPickupAmmo);
        _v.Field("rocketPickupAmmo", _t.weapons.rocketPickupAmmo);
        _v.Field("flamePickupAmmo", _t.weapons.flamePickupAmmo);
        _v.End();

        _v.Begin("knife");
        _v.Field("range", _t.knife.range);
        _v.Field("damage", _t.knife.damage);
        _v.Field("reach", _t.knife.reach);
        _v.Field("back", _t.knife.back);
        _v.Field("bottom", _t.knife.bottom);
        _v.Field("top", _t.knife.top);
        _v.Field("durationTicks", _t.knife.durationTicks);
        _v.Field("activeFirstTick", _t.knife.activeFirstTick);
        _v.Field("activeLastTick", _t.knife.activeLastTick);
        _v.Field("chainWindowTicks", _t.knife.chainWindowTicks);
        _v.End();

        _v.Begin("bomb");
        _v.Field("initial", _t.bomb.initial);
        _v.Field("maxInFlight", _t.bomb.maxInFlight);
        _v.Field("throwTicks", _t.bomb.throwTicks);
        _v.Field("releaseTick", _t.bomb.releaseTick);
        _v.Field("releaseOffset", _t.bomb.releaseOffset);
        _v.Field("speedX", _t.bomb.speedX);
        _v.Field("speedY", _t.bomb.speedY);
        _v.Field("fuseTicks", _t.bomb.fuseTicks);
        _v.Field("inheritMoveX", _t.bomb.inheritMoveX);
        _v.Field("gravity", _t.bomb.gravity);
        _v.Field("hitSize", _t.bomb.hitSize);
        _v.Field("damage", _t.bomb.damage);
        _v.Field("blastSize", _t.bomb.blastSize);
        _v.Field("blastUp", _t.bomb.blastUp);
        _v.Field("blastTicks", _t.bomb.blastTicks);
        _v.End();

        _v.Begin("player");
        _v.Field("lives", _t.player.lives);
        _v.Field("deathTicks", _t.player.deathTicks);
        _v.Field("respawnInvincible", _t.player.respawnInvincible);
        _v.Field("blinkPeriodTicks", _t.player.blinkPeriodTicks);
        _v.Field("respawnOffsetX", _t.player.respawnOffsetX);
        _v.Field("respawnDropY", _t.player.respawnDropY);
        _v.Field("gameOverTicks", _t.player.gameOverTicks);
        _v.Field("idleActionTicks", _t.player.idleActionTicks);
        _v.Field("idleActionLength", _t.player.idleActionLength);
        _v.End();

        _v.Begin("camera");
        _v.Field("deadzoneHalf", _t.camera.deadzoneHalf);
        _v.Field("smoothTime", _t.camera.smoothTime);
        _v.Field("lookAheadX", _t.camera.lookAheadX);
        _v.End();

        _v.Begin("sandbox");
        _v.Field("gruntHp", _t.sandbox.gruntHp);
        _v.Field("jeepHp", _t.sandbox.jeepHp);
        _v.Field("busHp", _t.sandbox.busHp);
        _v.Field("dummyRespawnTicks", _t.sandbox.dummyRespawnTicks);
        _v.Field("shooterIntervalTicks", _t.sandbox.shooterIntervalTicks);
        _v.Field("enemyBulletSpeed", _t.sandbox.enemyBulletSpeed);
        _v.Field("enemyBulletDamage", _t.sandbox.enemyBulletDamage);
        _v.Field("shooterRange", _t.sandbox.shooterRange);
        _v.End();

        _v.Begin("rig");
        _v.Field("airBaseline", _t.rig.airBaseline);
        _v.Field("diagUpVariant", _t.rig.diagUpVariant);
        _v.End();
    }

    class ReadVisitor
    {
    public:
        explicit ReadVisitor(
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
            bool&            _value)
        {
            const std::optional<JsonReader>& top = m_stack.back();
            if (!top || !top->HasField(_key))
            {
                return;
            }
            const Result<bool> value = top->ReadField<bool>(_key);
            if (!value)
            {
                m_bError = true;
                return;
            }
            _value = value.GetValue();
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

    class DumpVisitor
    {
    public:
        void Begin(
            const StringView _key)
        {
            m_path.push_back(String { _key });
        }

        void End()
        {
            m_path.pop_back();
        }

        void Field(
            const StringView _key,
            const float      _value)
        {
            Line_(_key, std::format("{:.4f}", _value));
        }

        void Field(
            const StringView _key,
            const int        _value)
        {
            Line_(_key, std::to_string(_value));
        }

        void Field(
            const StringView _key,
            const bool       _value)
        {
            Line_(_key, _value ? "true" : "false");
        }

        void Field(
            const StringView _key,
            const Vec2       _value)
        {
            Line_(_key, std::format("[{:.4f}, {:.4f}]", _value.x, _value.y));
        }

        [[nodiscard]] const String& GetText() const { return m_text; }

    private:
        void Line_(
            const StringView _key,
            const String&    _value)
        {
            for (const String& part: m_path)
            {
                m_text += part;
                m_text += '.';
            }
            m_text += _key;
            m_text += " = ";
            m_text += _value;
            m_text += '\n';
        }

        Vector<String> m_path = {};
        String         m_text = {};
    };

    [[nodiscard]] Result<Tuning> ParseRoot_(
        const JsonReader& _root)
    {
        Tuning      tuning;
        ReadVisitor visitor { _root };
        Visit_(visitor, tuning);
        if (visitor.HasError())
        {
            return Error { eError::Parse };
        }
        return tuning;
    }

}   // namespace

Result<Tuning> ParseTuning(
    const StringView _json)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json, eJsonLoadOption::AllowComments);
    if (!doc)
    {
        return Error { eError::Parse };
    }
    return ParseRoot_(doc->GetReader());
}

Result<Tuning> LoadTuning(
    const FilePath& _path)
{
    Result<JsonDeserializer> doc = JsonDeserializer::LoadFromFile(_path, eJsonLoadOption::AllowComments);
    if (!doc)
    {
        return Error { eError::FileOpen };
    }
    return ParseRoot_(doc->GetReader());
}

String DumpTuning(
    const Tuning& _tuning)
{
    DumpVisitor visitor;
    Visit_(visitor, _tuning);
    return visitor.GetText();
}

FilePath GetDefaultTuningPath(
    const FilePath& _assetRoot)
{
    return _assetRoot / "Tuning" / "Tuning.json";
}

void TuningWatcher::Init(
    FilePath  _path,
    const int _pollIntervalTicks)
{
    m_path        = std::move(_path);
    m_interval    = _pollIntervalTicks < 1 ? 1 : _pollIntervalTicks;
    m_counter     = 0;
    m_reloadCount = 0;
    m_bHasStamp   = false;

    std::error_code          ec;
    const fs::file_time_type stamp = fs::last_write_time(m_path, ec);
    if (!ec)
    {
        m_stamp     = stamp;
        m_bHasStamp = true;
    }
}

bool TuningWatcher::Poll(
    Tuning& _out)
{
    if (++m_counter < m_interval)
    {
        return false;
    }
    m_counter = 0;

    std::error_code          ec;
    const fs::file_time_type stamp = fs::last_write_time(m_path, ec);
    if (ec)
    {
        return false;
    }
    if (m_bHasStamp && stamp == m_stamp)
    {
        return false;
    }
    m_stamp     = stamp;
    m_bHasStamp = true;

    Result<Tuning> loaded = LoadTuning(m_path);
    if (!loaded)
    {
        SLUG_LOG_WARN("Tuning 리로드 실패 (이전 값 유지): {}", m_path.string());
        return false;
    }
    _out = loaded.Take();
    ++m_reloadCount;
    SLUG_LOG_INFO("Tuning 리로드: {}", m_path.string());
    return true;
}

}   // namespace slug
