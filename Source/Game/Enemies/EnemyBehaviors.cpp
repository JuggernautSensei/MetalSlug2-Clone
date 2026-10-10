#include "Game/Enemies/EnemyBehaviors.h"

namespace slug
{
const BehaviorOps& GetBehaviorOps(
    const eEnemyBehavior _behavior)
{
    switch (_behavior)
    {
        case eEnemyBehavior::Soldier: return kSoldierOps;
        case eEnemyBehavior::Bzk: return kBzkOps;
        case eEnemyBehavior::Hang: return kHangOps;
        case eEnemyBehavior::FlyingBzk: return kFlyingBzkOps;
        case eEnemyBehavior::Shielder: return kShielderOps;
        case eEnemyBehavior::Pow: return kPowOps;
        case eEnemyBehavior::Bus: return kBusOps;
        case eEnemyBehavior::BossShiee: return kBossShieeOps;
        case eEnemyBehavior::Jeep: return kJeepOps;
        case eEnemyBehavior::BTank: return kBTankOps;
        case eEnemyBehavior::MissleTank: return kMissleTankOps;
        case eEnemyBehavior::Sarubia: return kSarubiaOps;
        case eEnemyBehavior::Aparch: return kAparchOps;
        case eEnemyBehavior::Count: break;
    }
    static const BehaviorOps kEmpty {};
    return kEmpty;
}
}
