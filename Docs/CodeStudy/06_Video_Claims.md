# 06. 영상 자막의 코드 근거

프로토타입 영상(`Video/edit/timeline.json`)의 자막 한 줄마다 그 말을 뒷받침하는 코드 위치를 적었습니다. 영상에서 한 말을 질문받았을 때 바로 열어서 설명할 수 있게 하는 것이 목적입니다. 플레이어부터 보스까지 한 틱이 흐르는 과정은 [02 §3 실제 틱 순서](02_Gameplay_Combat_Enemies.md)와 [02 §8.14 BigShiee](02_Gameplay_Combat_Enemies.md)에서, 에디터의 실시간 반영 범위는 [07 §10 hot reload 표](07_Tools_Data_Audio_DeepDive.md)에서 이어집니다.

기준은 2026-10-14 작업 폴더의 Source입니다. 녹화 영상은 그 이전에 실행한 결과이므로, 화면에 보이는 UI 문구와 현재 코드가 조금 다를 수 있습니다. 시간은 timeline.json을 고치면 달라집니다.

## 1. 플레이 롱테이크 (Source2 133~399초)

| 시간 | 자막 | 근거 | 설명할 때 주의 |
|---|---|---|---|
| 0:22 | 고정 60Hz 틱, 화면은 보간 | `Source/Core/FixedStep.h:6` `kStepSec = 1/60` · `FixedStep.cpp:10` 누적 시간 소비 · `App.cpp:480` `Render_(m_clock.GetAlpha())` | 한 프레임에 처리하는 최대 스텝 수가 있음(`kMaxStepsPerFrame`). 너무 느려지면 시간을 버림 |
| 0:40 | 22개 섹션, triggerX를 넘으면 실행 | `Assets/Stage/Stage0.json` sections · `Source/Game/Stage/StageData.cpp:1801` 파싱, `:1820` triggerX 오름차순 검사 | 섹션의 actions는 spawn만 있는 게 아님(fade, banner 등 17종 — [03 §3.2](03_Editor_Data_Audio.md)) |
| 0:58 | Ground 마스크 PNG 픽셀 검사 | `Source/World/GroundMap.cpp:287` `IsSolid` · `:157` `IsRectSolid` · `Assets/Ground/Stage0_0.png` | 마스크는 맵마다 따로 있고 스테이지 진행 중에 바뀜(`ground.maps`, ground_swap 섹션) |
| 1:18 | 행동은 파생 클래스, 수치는 Defs JSON | `Source/Game/Enemies/SoldierEnemy.h:21` `class SoldierEnemy : public Enemy` · `Assets/Defs/Helicopter.json` (speed, box, anim) | 클래스는 생성 시 한 번 선택([02 §7.1](02_Gameplay_Combat_Enemies.md)) |
| 1:43 | boundsCurves로 카메라 경계 보간 | `Stage0.json` camera.boundsCurves `downtown_slope` · `Source/Game/Stage/StageRunner.cpp:334~340` `EvaluateBounds(playerX)` | 카메라 이동 자체는 단순 Lerp가 아님([01 §9](01_Core_Render_World.md)) |
| 2:05 | Hitbox × Hurtbox, hit ID 중복 방지 | `Source/Game/Combat/CombatSystem.cpp:5` BeginTick · `:54` `NewHitId` · `:59` `ReleaseHitId` | 매 틱 지우는 건 등록 배열뿐. 같은 공격의 피해 기록은 hit ID를 해제할 때까지 남음 |
| 2:30 | depth → sortKey → 제출 순서 | `Source/Render/SpriteBatch.cpp:50~62` 정렬 비교 함수 · `:755` `seq = m_nextSeq++` | 단순 Y 정렬이 아님 |
| 3:00 | 몸체·안테나·주포·포탑 7기 조립 | `Source/Game/Enemies/BigShieeConfig.h:4` `kBigShieeTurretCount = 7` · `BigShiee.cpp:578` `SubmitParts` (hull→antenna→turrets→cannon→casing) | 제출 순서와 최종 그림 순서는 다름. 최종 순서는 depth가 결정 |
| 3:25 | 포탑마다 HP·모드, 전부 부서지면 주포 | `BigShiee.h:33~36` turretHp/Mode/Ticks/Position · `BigShiee.cpp:426` `OnHit`, `:437` 포탑 HP 감소 · [02 §8.14 전개와 주포](02_Gameplay_Combat_Enemies.md) | 보스 본체 체력은 주포가 노출됐을 때만 줄어듦(`CannonVulnerable_`) |
| 3:52 | VoiceAllocator가 같은 소리 동시 재생 제한 | `Source/Audio/VoiceAllocator.h` (기본값 `m_maxPerSound = 4`) · `VoiceAllocator.cpp:25` 같은 소리가 넘치면 가장 오래된 것 교체 · `:40` 전체 슬롯이 차면 가장 오래된 것 교체 | 장치 출력은 `AudioSystem.cpp`의 miniaudio |

## 2. 에디터 (Source3)

| 시간 | 자막 | 근거 | 설명할 때 주의 |
|---|---|---|---|
| 4:32 | 프레임별 pivotX·pivotY·frameDuration | `Source/Editor/AnimEditor.cpp:368~383` pivot 입력, Apply to all · `AnimDocument` | 문서 안에서 고친 값은 저장 전에도 AnimationLibrary에 적용됨([07 §10](07_Tools_Data_Audio_DeepDive.md)) |
| 4:43 | 화살표 키 1px, onion skin | `AnimEditor.cpp:522~526` Left/Right 화살표 · `:339` onion 체크박스 | |
| 4:55 | 저장, 외부 변경 시 다시 읽되 미저장 편집 보존 | `AnimEditor.cpp:94~105` `CheckExternal` — dirty면 다시 읽지 않고 상태 문구만 남김 | atlas 텍스처는 자동으로 새로 고치지 않음 |
| 5:07 | 섹션과 triggerX를 화면 위에서 확인 | `Source/Editor/StageEditor.cpp` · `StageDocument` | Map 편집은 **실행 중인 스테이지를 자동으로 바꾸지 않음**. 저장한 JSON을 다음 로드 때 사용 |
| 5:20 | Place mode, X·Y·groundSnap·Z | `StageEditor.cpp:927` Place mode · `:945` groundSnap 안내 | |
| 5:35 | 위치를 바꿔도 다시 빌드하지 않음 | `StageDocument` 원문 span 패치 + 전체 텍스트 Undo ([03 §5.2](03_Editor_Data_Audio.md)) | "빌드 없이"가 정확한 표현. "실행 중 즉시 반영"이라고 말하면 틀림 |
| 5:57 | 연필·지우개·사각형·직선·채우기 | `Source/Editor/GroundEditor.h:36~38` `DrawLine`/`FillRect`/`FloodFill`, `:62` 도구 enum | Ground PNG도 실행 중인 GroundMap을 자동으로 바꾸지 않음 |
| 6:31~6:57 | 부위별 스프라이트, 위치·타이밍 맞춤, Depth 표 | `Source/Editor/DepthEditor.cpp:95~97` BigShiee hull/antenna/cannon 상수 행 · [07 §11 보스 QA 루프](07_Tools_Data_Audio_DeepDive.md) | 그림 시간(clip)과 공격 틱(config)은 서로 다른 설정 |
| 7:13 | Pause·Step 1·Step 10·0.25x | `Source/Editor/EnemyLab.cpp:357` Step 1 · `:363` Step 10 | 샌드박스(AI 탭)에서만 사용 |
| 7:27 | muzzle 오프셋, Fire now | `Source/Editor/MotionEditor.cpp:443` Fire now · `:27` muzzle | Motion params는 값을 바꾸면 바로 파일에 씀([07 §8.4](07_Tools_Data_Audio_DeepDive.md)) |
| 7:43 | 몸통·피격 상자만 바로 적용 | `Source/Editor/HitboxEditor.cpp:117` `ApplyLive_` — standSize/crouchSize/hurtStand/hurtCrouch, Enemy body/hurtbox, BigShiee cannonBox만 | 무기 hitSize 같은 나머지 값은 Tuning 저장 → 감시 재로드 |
| 7:56 | 샌드박스가 Tuning.json 감시, 실패 시 이전 값 | `Source/Core/Tuning.cpp:471` `TuningWatcher::Poll` (mtime 비교, 실패하면 경고 후 유지) · `Source/Game/SandboxState.cpp:466` | StageState에는 같은 Poll이 없음 |
| 8:09 | 스폰·전부 제거·마지막 세트 다시 스폰 | `EnemyLab.cpp:452` Kill all · `:462` Respawn last set | |
| 8:35 | Defs 변경 시 적을 유지한 채 재로드 | `EnemyLab.cpp:176~203` `PollDefFiles_` (60프레임마다 Defs 폴더 최대 mtime) → `:146` `ReloadDefsKeep` | 호환 조건이 안 맞으면 전체 재로드 + 세트 재스폰. 그래서 자막에 "최대한" |

## 3. 회고 문장 (트레일러 위)

이 문장들은 코드가 아니라 사용자가 직접 정리한 개발 경험입니다(대화 기록 2026-10-13). 영상에서는 이 경험을 그대로 쓰고, 기능에 대한 말은 위 표의 범위를 넘지 않게 했습니다.

- "데이터로 바꾸자, 1px 수정에 빌드가 필요 없어졌다" — 위치·스펙·충돌을 **데이터로** 고치고 빌드 없이 확인. 실행 중 즉시 반영되는 범위는 애니메이션, Tuning(샌드박스), Defs(샌드박스), Hitbox 일부.
- "AI는 계획이 구체적일수록 잘 만든다" — 계획, 테스트 준비, 검증 루프.
- "QA 환경은 함께 만들고, 손맛은 직접 확인" — AI로 QA 환경(Lab, Step, overlay)을 만들고, QA는 직접 함.

## 4. 영상에서 일부러 하지 않은 말

- 보스의 모든 부위에 독립 체력이 있다: 포탑 7기에만 각각 HP가 있고, 몸체와 안테나에는 없음.
- 맵·마스크 편집이 실행 중인 게임에 즉시 반영된다: 자동으로 교체하는 경로가 없음.
- 결정적 재현이 완전하다: Hash/checkpoint 일치는 전체 동치의 증명이 아님(README 주의 사항).
