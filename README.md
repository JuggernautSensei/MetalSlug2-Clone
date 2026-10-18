<div align="center">

# MetalSlug2-Clone

**C++과 SDL을 이용한 메탈슬러그2 모작**

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white)
![Windows x64](https://img.shields.io/badge/Windows-x64-0078D6)
![SDL3](https://img.shields.io/badge/SDL-3-183A61)
![Dear ImGui](https://img.shields.io/badge/Dear_ImGui-Editor-38BFA3)

<img src="Media/banner.jpg" alt="메탈슬러그 2 모작의 보스전과 개발 화면" width="960">

</div>

<br>

## Contents

- [프로젝트 개요](#overview)
- [조작과 전투](#player)
- [충돌과 지형](#collision)
- [적과 보스전](#enemies)
- [JSON 데이터와 스테이지](#data)
- [카메라](#camera)
- [이미지 로딩 최적화](#loading)
- [ImGui 편집기](#editors)
- [게임 루프와 화면·오디오](#runtime)
- [빌드 및 실행](#build)

<a id="overview"></a>
## 프로젝트 개요

<p align="center"><img src="Media/hero.gif" alt="시가지에서 지형을 따라 이동하고 적과 싸우는 플레이 장면" width="840"></p>

메탈슬러그 2의 횡스크롤 전투를 C++20과 SDL3로 구현한 프로젝트입니다. 달리기·앉기·점프와 공격부터 적의 행동, 파괴 가능한 배경 오브젝트, 포로와 아이템, 보스전과 결과 화면까지 하나의 플레이 흐름으로 연결했습니다. 애니메이션·스테이지·지형 충돌·AI를 수정하는 ImGui 편집기도 같은 실행 파일에서 사용할 수 있습니다.

게임은 `MISSION 4` 배너로 시작해 시가지, 버스 전투, 차량과 공중 적이 등장하는 구간을 지나 Big Shiee 보스전으로 이어집니다. 원작 전체를 구현한 프로젝트는 아닙니다.

<a id="player"></a>
## 조작과 전투

<p align="center"><img src="Media/combat.gif" alt="앉기와 점프 중 사격, 병사와 아이템을 포함한 시가지 전투" width="760"></p>
<p align="center"><sub>이동·자세에 따라 공격 동작과 총구가 달라지고, 발사체의 명중은 공통 전투 판정으로 이어집니다.</sub></p>

달리면서 위를 쏘는 캐릭터에게는 이동, 자세, 조준 방향, 무기와 공격 동작이 동시에 필요합니다. 이 조합을 하나의 상태 이름에 모두 담으면 무기와 공격 방향마다 상태가 늘어나므로, 정보를 나누어 계산한 뒤 상체와 하체 클립을 따로 골라 조합했습니다.

| 단계 | 담당 | 하는 일 |
|---|---|---|
| 입력 | `PlayerInputBuffer` | 점프·사격·수류탄 입력을 잠시 보관, 코요테 타임 |
| 조작 | `PlayerController` | 이동 → 조준 → 기본 상태 도출 → 공격 순으로 갱신 |
| 상태 | `RigState` | 자세·좌우·조준 방향·무기와 공격 오버레이 전달 |
| 표현 | `PlayerRig` | 상하체 클립 선택, 걷는 상하체의 재생 위상 동기화 |

무기는 `WeaponState`가 탄약과 발사 간격을 관리하고, 생성된 탄은 `ProjectileSystem`이 이어받습니다. 공격력·사거리·판정 크기는 [`Tuning.json`](Assets/Tuning/Tuning.json)에서 조정합니다.

| 무기 | 특징 |
|---|---|
| 권총 | 탄약 무제한, 발사 간격과 동시 탄 수 제한 |
| HMG | 버스트 사격, 조준이 바뀌는 동안 중간 각도로도 발사 |
| 샷건 | 산탄 대신 짧은 사거리·큰 판정의 관통 공격 하나 |
| 로켓 | 가속하며 이동, 명중하면 별도의 폭발 판정으로 전환 |
| 화염방사기 | 입자를 시간차로 분출, 공격 ID를 공유해 피해 중복 방지 |
| 칼 | 근접 대상이 감지 범위에 있으면 사격 대신 전환 |
| 수류탄 | 지정 행동 틱에 생성, 중력·튕김·신관·폭발 규칙 적용 |

관련 구현: [`PlayerController.cpp`](Source/Game/Player/PlayerController.cpp), [`PlayerRig.cpp`](Source/Game/Player/PlayerRig.cpp), [`PlayerAttack.cpp`](Source/Game/Player/PlayerAttack.cpp), [`PlayerWeapons.cpp`](Source/Game/Player/PlayerWeapons.cpp), [`Projectiles.cpp`](Source/Game/Combat/Projectiles.cpp).

<a id="collision"></a>
## 충돌과 지형

<p align="center">
  <img src="Media/ai-inspector.png" alt="적의 공격·피격 범위와 추적 대상선을 표시한 화면" width="49%">
  <img src="Media/collision-editor.png" alt="배경 위에 빨간 고체 마스크를 겹쳐 본 지형 편집 화면" width="49%">
</p>
<p align="center"><sub>왼쪽은 적과 공격의 AABB 판정 범위, 오른쪽은 배경 위에 겹친 지형 고체 마스크(빨간 영역)입니다.</sub></p>

충돌은 전투 판정과 지형 이동으로 나누었습니다. 같은 월드에서 두 방식을 사용하지만, 각각이 판단하는 대상과 결과는 구분됩니다.

| 구분 | 데이터 | 처리 |
|---|---|---|
| 전투 판정 | AABB `Hitbox`·`Hurtbox` | 겹침과 팀·무적·공격 종류를 확인해 `HitEvent` 생성 |
| 지형 이동 | `GroundMap` 1비트 픽셀 마스크 | 이동을 작은 구간으로 나눠 벽·천장·바닥 검사, 단차와 경사 추적 |

여러 틱 동안 남는 공격은 공격 ID로 명중 이력을 관리해 같은 대상에게 매 틱 피해를 주지 않습니다. 빠른 발사체는 이전·현재 위치를 덮는 상자와 이동 선분 샘플링으로 얇은 지형을 건너뛰지 않게 했습니다. 파괴 가능한 오브젝트는 파괴되면 충돌 목록을 다시 만들고, 지정된 경우 지형 일부를 제거합니다.

관련 구현: [`Combat.cpp`](Source/World/Combat.cpp), [`GroundMap.cpp`](Source/World/GroundMap.cpp), [`Physics.cpp`](Source/World/Physics.cpp), [`PropWorld.cpp`](Source/Game/Props/PropWorld.cpp).

<a id="enemies"></a>
## 적과 보스전

적은 체력·활성화·피격·사망과 제거라는 공통 흐름을 갖지만, 다음 행동을 고르는 판단은 종류마다 다릅니다. 공통 흐름은 `EnemyWorld`가, 수치와 자원은 JSON의 `EnemyDef`가, 종류별 판단은 `Enemy` 파생 클래스의 `Tick`·`OnHit`·`OnDeath`가 맡습니다.

| 종류 | 행동 |
|---|---|
| 일반 병사 | 수류탄·칼·미사일형별 접근·대기·후퇴·공격, 벽 점프와 발판 하강 |
| 방패병 | 방패와 몸통 판정을 따로 등록해 방어, 사격 중에는 방패를 내림 |
| 차량 | 지프·BTank·미사일 탱크·Sarubia, 이동과 발사 준비를 상태로 연결 |
| 공중 | 낙하산 바주카병, 헬기의 폭탄 버스트와 미사일 일제사격 |

<p align="center">
  <img src="Media/bus.gif" alt="피해 단계에 따라 변하는 버스와 파괴 후 폭발·파편·잔해" width="49%">
  <img src="Media/boss.gif" alt="Big Shiee의 포탑·주포와 다중 폭발이 이어지는 최종 보스전" width="49%">
</p>
<p align="center"><sub>왼쪽은 병사를 내보내다 파괴되는 버스, 오른쪽은 포탑전에서 주포전으로 넘어가는 Big Shiee 보스전입니다.</sub></p>

버스는 문과 계단을 전개해 병사를 내보내고, 체력 소진과 파괴 연출 완료를 구분합니다. 스테이지는 연출 완료 신호를 기다린 뒤 카메라 잠금을 풀고 다음 구간으로 진행합니다. Big Shiee는 입장 → 포탑전 → 주포 전개 → 주포전으로 진행하며, 포탑과 주포에 서로 다른 소유자 식별자를 붙여 공통 전투 이벤트에서 맞은 부위를 찾아냅니다.

관련 구현: [`EnemyWorld.cpp`](Source/Game/Enemies/EnemyWorld.cpp), [`EnemyCommon.cpp`](Source/Game/Enemies/EnemyCommon.cpp), [`Bus.cpp`](Source/Game/Enemies/Bus.cpp), [`BigShiee.cpp`](Source/Game/Enemies/BigShiee.cpp).

<a id="data"></a>
## JSON 데이터와 스테이지

스테이지를 조정할 때마다 적 생성 코드와 카메라 코드를 함께 바꾸지 않도록 진행 구성을 JSON으로 분리했습니다. 플레이어의 X좌표가 섹션을 넘으면 스폰과 웨이브, 카메라 잠금·이동, 배경·BGM 교체, 배너·페이드 액션을 실행합니다. 게이트는 지정 개체의 격파나 파괴 연출 완료를 기다리므로, 버스가 파괴될 때까지 전투 구역을 유지하는 흐름도 데이터로 표현합니다.

적 정의는 `extends`로 공통 부분을 상속합니다. `human_base`에 보병의 몸체와 사망 클립을, `soldier_base`에 이동과 기본 클립을 두고, `soldier_bomb`는 공격 파라미터와 투척 클립, 발사체만 지정합니다.

```json
{
  "soldier_bomb": {
    "extends": "soldier_base",
    "params": { "ai": "bomb", "detect": 350, "closeStop": 200, "fireFrame": 10 },
    "clips": {
      "throw": { "name": "SD_Throw_Bomb_{side}", "frames": 14, "sec": 0.08 }
    },
    "weapons": {
      "attack": { "projectile": "soldier_bomb_proj", "muzzle": [0, -37], "aim": "facing" }
    }
  }
}
```

| 파일 | 내용 |
|---|---|
| [`Stage0.json`](Assets/Stage/Stage0.json) | 시작 위치, 배경·지형, 카메라 영역, 섹션 트리거와 액션 |
| [`Enemies.json`](Assets/Defs/Enemies.json) | 적의 몸체·체력·클립·무기·행동 파라미터 |
| [`Props.json`](Assets/Defs/Props/Props.json) | 배경 오브젝트의 손상 단계, 충돌 역할, 파편과 드롭 |
| [`Tuning.json`](Assets/Tuning/Tuning.json) | 이동·점프·입력 버퍼, 판정 크기, 무기·카메라 설정 |
| `anim.json` · `sprites.json` · `sounds.json` | 클립과 프레임·피벗, 스프라이트, 사운드 연결 |

관련 구현: [`StageRunner.cpp`](Source/Game/Stage/StageRunner.cpp), [`EnemyDefLoader.cpp`](Source/Game/Enemies/EnemyDefLoader.cpp).

<a id="camera"></a>
## 카메라

플레이어 위치를 그대로 카메라 중심에 대입하면 작은 움직임까지 화면 전체에 전달됩니다. 그래서 목표 주변에 데드존을 두고, 그 밖으로 나갔을 때 추적합니다.

| 상황 | 동작 |
|---|---|
| 데드존 안 | 작은 움직임을 흡수하고 화면을 유지 |
| 데드존 밖 | 이동량에 따라 가속하고, 멈추거나 반대로 움직이면 감속 |
| 목표와 크게 벌어짐 | 글라이드 상태로 가속·감속하며 따라잡음 |
| 지정 위치 이동 | 기존 속도를 이은 3차 보간으로 이동한 뒤 잠금 |
| 전투 구역·보스전 | 잠금 영역 안에서만 이동, 보스전은 잠금 영역을 옮겨 자동 스크롤 |

관련 구현: [`Camera.cpp`](Source/Render/Camera.cpp), [`StageState.cpp`](Source/Game/Stage/StageState.cpp).

<a id="loading"></a>
## 이미지 로딩 최적화

이미지 로딩은 파일을 읽고 PNG를 RGBA로 푸는 작업과, 그 결과로 SDL 텍스처를 만드는 작업으로 나눴습니다. 아틀라스에는 피격 시 사용할 색상 변환본도 필요하므로 디코딩 결과에서 해당 RGBA를 함께 만듭니다. 이 파일 읽기·디코딩·색상 변환을 이미지별 독립 작업으로 처리하고, 렌더러를 사용하는 텍스처 생성은 호출 스레드에서 이어갑니다.

`DecodeImages`는 미리 확보한 결과 배열의 각 위치에 병렬 디코딩 결과를 기록합니다. 작업이 끝난 뒤에는 경로의 원래 순서대로 아틀라스를 추가합니다. 디코딩이 먼저 끝난 이미지부터 등록하지 않기 때문에 애니메이션 프레임이 참조하는 페이지 번호를 유지할 수 있습니다.

한 번에 모든 이미지의 원본·변환 RGBA를 보관하지 않도록 경로를 묶음으로 나누고, 각 묶음의 디코딩과 텍스처 등록이 끝나면 다음으로 넘어갑니다. 기본 묶음 크기는 하드웨어 동시 실행 수에서 하나를 뺀 값을 1~8로 제한한 설정을 사용합니다. `--no-async-load`를 주면 같은 경로를 순차 디코딩합니다.

같은 Release 실행 파일에서 `--no-async-load`의 유무만 바꾸어 초기 실행 시간을 비교했습니다. 모드별 예열 1회는 제외하고, 순서를 번갈아 5회씩 실행한 중앙값입니다. 프로세스 시작부터 스테이지의 첫 프레임 BMP를 저장하고 종료할 때까지를 재므로, 초기화·이미지 로딩·렌더링·캡처·정리가 모두 포함됩니다. 오디오와 편집기는 사용하지 않았고 파일 캐시가 데워진 상태의 비교입니다.

<p align="center"><img src="Media/loading-comparison.png" alt="순차 모드 23.34초와 병렬 모드 12.21초의 초기 실행 시간 중앙값 비교" width="960"></p>
<p align="center"><sub>동일한 실행 파일에서 순차·병렬 모드를 교차 실행한 결과입니다. 첫 프레임 저장과 종료까지 포함합니다.</sub></p>

| 실행 모드 | 5회 중앙값 |
|---|---:|
| 순차 디코딩 (`--no-async-load`) | 23.34초 |
| 병렬 디코딩 (기본 모드) | 12.21초 |

측정 범위 전체에서 약 **47.7% 짧아졌으며**, 시간 비율로는 약 **1.91배**입니다. PNG 디코딩 함수만 분리해서 잰 수치는 아닙니다. 예열을 포함한 12회 실행에서 텍스처 20개의 로드 순서·크기·RGBA 해시가 모두 같아, 이 로딩 경로가 같은 이미지 데이터를 같은 순서로 등록하는 것도 함께 확인했습니다.

> 측정 환경: 2026-10-18, AMD Ryzen 9 7945HX(16코어·32논리 프로세서), Windows 빌드 26200, x64 Release, 기본 묶음 설정 8. [반복별 원시 결과와 측정 조건](Media/loading-benchmark.json)을 함께 남겼습니다. 다른 저장장치·캐시 상태·컴퓨터에서는 시간이 달라질 수 있습니다.

긴 배경은 로딩 이후 표시 방식도 다릅니다. 가로 길이가 큰 이미지는 폭 2048 이하의 텍스처 조각으로 나누고, 카메라에서 보이는 구간에 해당하는 부분을 그립니다.

관련 구현: [`SpriteBatch.cpp`](Source/Render/SpriteBatch.cpp), [`App.cpp`](Source/Core/App.cpp), [`StageState.cpp`](Source/Game/Stage/StageState.cpp).

<a id="editors"></a>
## ImGui 편집기

편집기는 Animation, Map, Collision, AI 네 탭으로 구성했습니다. 파일을 저장하는 기능뿐 아니라 수정 대상과 실제 화면을 함께 보는 데 초점을 맞췄습니다.

<table>
  <tr>
    <td width="50%"><img src="Media/editor_anim.gif" alt="애니메이션 클립과 프레임 피벗을 확인하고 어니언 스킨으로 동작을 비교하는 편집 화면" width="100%"></td>
    <td width="50%"><img src="Media/editor_map.gif" alt="지도에서 개체를 배치하고 스테이지 섹션을 살펴보는 맵 편집기" width="100%"></td>
  </tr>
  <tr>
    <td><b>Animation</b> · 프레임 시간과 피벗을 수정하고, 어니언 스킨으로 인접 프레임의 연결을 확인</td>
    <td><b>Map</b> · 개체를 배치하고, 섹션 트리거와 카메라 영역·잠금 시간을 조정</td>
  </tr>
  <tr>
    <td><img src="Media/editor_collision.gif" alt="배경 지형 위에서 고체 영역을 그리고 지우는 픽셀 충돌 편집기" width="100%"></td>
    <td><img src="Media/editor_ai.gif" alt="여러 적을 소환하고 상태·판정·대상선을 표시해 AI를 확인하는 장면" width="100%"></td>
  </tr>
  <tr>
    <td><b>Collision</b> · 연필·지우개·영역 채우기로 고체 마스크를 편집</td>
    <td><b>AI</b> · 적을 소환해 상태·판정 상자·대상선을 보며 틱 단위로 확인</td>
  </tr>
</table>

AI 탭에는 히트박스·모션·깊이 도구도 있어 판정 크기와 공격 유효 프레임, 총구 위치, 그리기 순서를 수정하고 바로 시험해 볼 수 있습니다.

관련 구현: [`Editor.cpp`](Source/Editor/Editor.cpp), [`AnimEditor.cpp`](Source/Editor/AnimEditor.cpp), [`StageEditor.cpp`](Source/Editor/StageEditor.cpp), [`GroundEditor.cpp`](Source/Editor/GroundEditor.cpp), [`EnemyLab.cpp`](Source/Editor/EnemyLab.cpp), [`HitboxEditor.cpp`](Source/Editor/HitboxEditor.cpp), [`MotionEditor.cpp`](Source/Editor/MotionEditor.cpp), [`DepthEditor.cpp`](Source/Editor/DepthEditor.cpp).

<a id="runtime"></a>
## 게임 루프와 화면·오디오

| 항목 | 처리 |
|---|---|
| 게임 루프 | 60Hz 고정 틱, 남은 시간 비율로 플레이어·카메라·발사체 위치를 보간 |
| 상태 전환 | 타이틀·스테이지·샌드박스를 `IGameState`로 실행하고, `SimRunner`가 틱이 끝난 뒤 교체 |
| 렌더링 | 깊이·정렬 키·제출 순서로 정렬, 640×448 논리 해상도를 정수 배율·최근접 샘플링으로 표시 |
| HUD | 비트맵 글꼴과 스프라이트로 목숨·점수·탄약·수류탄·보스 체력 표시 |
| 오디오 | `sounds.json`의 이름으로 조회, 효과음은 디코딩·BGM은 스트리밍, 동시 재생 수 제한 |

관련 구현: [`FixedStep.cpp`](Source/Core/FixedStep.cpp), [`SimRunner.cpp`](Source/Game/SimRunner.cpp), [`SpriteBatch.cpp`](Source/Render/SpriteBatch.cpp), [`Hud.cpp`](Source/Game/Ui/Hud.cpp), [`AudioSystem.cpp`](Source/Audio/AudioSystem.cpp).

<a id="build"></a>
## 빌드 및 실행

Windows x64에서 C++20으로 빌드합니다. Visual Studio의 C++ 데스크톱 개발 도구와 Windows SDK, 프로젝트가 지정한 MSVC `v145` 도구 집합이 필요합니다. 의존성은 vcpkg manifest로 관리하며 SDL3, Dear ImGui, miniaudio, stb, yyjson 등을 사용합니다. JugX는 서브모듈로 가져와 정적 라이브러리로 연결합니다.

1. 서브모듈을 포함해 저장소를 받습니다.

   ```powershell
   git clone --recurse-submodules https://github.com/JuggernautSensei/MetalSlug2-Clone.git
   cd MetalSlug2-Clone
   ```

2. vcpkg의 경로를 `VCPKG_ROOT` 환경 변수로 지정합니다. 별도 지정이 없으면 프로젝트는 `C:\vcpkg\`를 사용합니다. 이미 받은 저장소라면 `git submodule update --init --recursive`로 서브모듈을 준비합니다.

3. `MetalSlug2_Clone.sln`을 열어 x64 Release로 빌드하거나 저장소 루트에서 스크립트를 실행합니다.

   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1 -Config Release
   .\Output\Release\MetalSlug2_Clone.exe
   ```

`Play.cmd`는 Release 증분 빌드를 호출한 뒤 게임을 실행합니다. 실행 파일만 옮기지 말고 `Assets`를 찾을 수 있는 저장소 구성을 유지해야 합니다. 빌드 로그는 `Intermediate/last_build.log`에 남습니다.

| 조작 | 키 |
|---|---|
| 이동·조준 | 방향키 또는 W/A/S/D |
| 사격·근접 공격 | J 또는 Z |
| 점프 | K 또는 X |
| 수류탄 | L 또는 C |
| 타이틀에서 시작 | Enter 또는 Space |
| 실행 종료 | Esc |

편집기는 다음과 같이 실행합니다.

```powershell
.\Output\Release\MetalSlug2_Clone.exe --editor
.\Output\Release\MetalSlug2_Clone.exe --editor --state enemylab --editor-tab ai
```

개발 중에는 F1로 판정을 표시하고 F2로 일시정지, F3으로 한 틱 진행, F4로 슬로모션을 켤 수 있습니다. `--warp-boss`나 `--warp-x`로 원하는 전투 구간부터 확인하고 `--capture`로 화면을 저장할 수 있습니다.

<br>

학습을 위한 비공식 모작입니다. 원작 게임과 사용한 그래픽·음원의 권리는 각 원저작권자에게 있습니다.
