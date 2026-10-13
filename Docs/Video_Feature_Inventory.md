# MetalSlug2 Clone — 영상용 기능·설계 목록

조사일: 2026-10-13. 현재 작업 트리의 정적 분석과 원본 영상 표본을 기준으로 작성.
이번 조사에서 게임을 빌드하거나 테스트를 실행하지 않았다. 구현 존재, 실제 시연 가능 여부,
개발 연도, 저자 경험, 성능 수치를 서로 구분한다. 기존 `Portfolio_Architecture.md`는 수정하지 않았다.

## 읽는 방법과 필터

- 분류: 기반 / 조작 / 물리·전투 / 적·보스 / 스테이지·렌더 / 에디터 / 사운드 / 검증.
- 영상: P1 전체 플레이 / P2 에디터 / P3 회고.
- 깊이: 핵심(자막 설명 권장) / 보조(짧게 소개) / 별도(문서 또는 후속 영상).
- 근거: 파일과 1-based 행 번호. 구현 존재를 확인한 위치이며 전체 함수의 정확성 증명은 아님.
- 연도 필터: 사용자 확인상 **2024 게임 전체 직접 구현 / 2026 에디터 제작(AI 보조)**.
  현재 소스에는 2026 리팩터링도 있으므로 개별 구조/함수의 최초 구현 연도를 이 구분만으로 확정하지 않는다.

## 현재 아키텍처

```text
App: SDL 이벤트, 프레임 시간, 고정 스텝, 렌더/오디오/에디터 조율
  InputFrame → InputHistory → SimRunner → IGameState
    StageState / SandboxState 등
      PlayerController + PlayerRig
      EnemyWorld → Enemy 파생 클래스
      PropWorld + ItemWorld + DecorWorld
      StageRunner → IStageHost / StageWorldHost
      Physics / GroundMap / CombatSystem / ProjectileSystem
    AnimationLibrary / SpriteLibrary → SpriteBatch → SDL3 Renderer
  Editor: 4개 상위 탭 + Document 데이터 편집 계층
  AudioSystem → SoundBank / VoiceAllocator / miniaudio
```

근거: `Source/Core/App.cpp:423`, `Source/Game/SimRunner.cpp:28`,
`Source/Game/Enemies/Enemy.h:53`, `Source/Game/Stage/StageWorldHost.h:65`.
입력과 시뮬레이션, 게임 상태, 렌더링, 제작 도구의 책임을 설명하는 구조이다.
ECS나 범용 엔진이라고 소개할 근거는 확인하지 않았다.

## 1. 기반·플레이어

| ID | 기능·설계 | 코드에서 확인한 내용 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| C01 | 고정 스텝 | 1/60초 갱신, 큰 dt 제한, 프레임당 최대 5틱, 초과 누적 시간 처리 | P1 보조 / P3 핵심 | `Source/Core/FixedStep.h:6`, `Source/Core/FixedStep.cpp:3` |
| C02 | 액션 입력 추상화 | SDL 키 상태를 이동·사격·점프·폭탄 등 액션 플래그로 샘플링 | P1 핵심 | `Source/Core/Input.h:8`, `Source/Core/Input.h:22` |
| C03 | 입력 이력 | 16프레임 입력 보관, Held와 Pressed 구분 | P1 핵심 | `Source/Core/Input.h:47` |
| C04 | 입력 버퍼 | 점프·사격·폭탄 입력을 일정 틱 기억하고 Consume으로 소비 | P1 핵심 | `Source/Game/Player/PlayerInput.cpp:8`, `:50` |
| C05 | 이동 상태와 명령 분리 | 좌우 동시 입력 처리와 방향/조준/공격 명령 구성; Idle/Run/Crouch/Jump 상태 유도 | P1 핵심 | `Source/Game/Player/PlayerInput.cpp:50`, `Source/Game/Player/PlayerMovement.cpp:248` |
| C06 | 조작 관용성 | 점프 버퍼와 코요테 타임; 둘은 다른 조건 | P1 보조 | `Source/Core/Tuning.h:25`, `Source/Game/Player/PlayerMovement.cpp:145` |
| C07 | 상·하체 애니메이션 | Upper/Lower AnimPlayer, 이동·조준·공격 오버레이, 상태/오버레이 serial | P1 핵심 / P2 핵심 | `Source/Game/Player/PlayerRig.h:13`, `:133` |
| C08 | 클립 선택 데이터 | 무기·좌우 방향·포즈·조준에 맞는 클립 슬롯 테이블과 누락 보고 | P1 핵심 | `Source/Game/Player/PlayerRig.h:95` |
| C09 | 무기 튜닝 | Pistol/Hmg/Shotgun/Rocket/Flame의 파라미터 및 획득 탄약 | P1 핵심 | `Source/Core/Tuning.h:7`, `Source/Core/Tuning.cpp:505` |
| C10 | 사망·복귀·승리 | 사망/Respawn/Victory 상태, 무적 시간과 낙하산 관련 처리 | P1 보조 | `Source/Game/Player/PlayerController.cpp:116`, `:253`, `:273` |
| C11 | 탑승 관련 처리 | PlayerController와 SlugSystem 계열 구현. 시연 분량·완성도는 추가 확인 | P1 보조 | `Source/Game/Player/PlayerController.cpp:389`, `Source/Game/Player/SlugSystem.cpp` |

## 2. 물리·전투

| ID | 기능·설계 | 코드에서 확인한 내용 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| W01 | 자작 2D 이동 물리 | Body 위치·속도, 중력/종단속도, X/Y 이동 처리, 착지 결과 | P1 핵심 | `Source/World/Physics.cpp:469` |
| W02 | 지형 충돌 마스크 | GroundMap 비트 데이터의 solid 판정과 지면 검색 | P1 핵심 / P2 핵심 | `Source/World/GroundMap.h:16` |
| W03 | 경사·접지 | 지면 재접지, 경사/법선 추정과 MoveResult 제공 | P1 보조 / P2 핵심 | `Source/World/Physics.cpp:314`, `:469` |
| W04 | 박스형 벽·발판 | AABB Platform/Wall, 단방향 발판과 dropThroughSteps | P1 핵심 | `Source/World/Physics.h:7`, `:25` |
| W05 | 공격과 피격 구분 | Hitbox/Hurtbox, 팀·무적·종류·소유자 검사 후 AABB 중첩 판정 | P1 핵심 / P2 핵심 | `Source/World/Combat.h:42`, `Source/World/Combat.cpp:17` |
| W06 | 중복 피격 방지 | hit ID별 이미 맞은 대상 기록; 비관통 공격 소비 | P1 보조 | `Source/World/Combat.cpp:32`, `:64` |
| W07 | 고속 탄환 판정 | 이전 위치와 현재 위치를 감싸는 SweptBox로 Hitbox 구성 | P1 핵심 | `Source/Game/Combat/Projectiles.cpp:67`, `:278` |
| W08 | 탄·폭탄·폭발 | 수명/이동/관통, 폭탄 fuse·접촉, 폭발 영역, 효과 요청 | P1 핵심 | `Source/Game/Combat/Projectiles.cpp:206`, `:444`, `:487` |
| W09 | 피격 시각 효과 | 타격 flash 텍스처 생성, 별도 효과/파편 경로 | P1 보조 | `Source/Render/SpriteBatch.h:68`, `Source/Game/Props/PropWorld.cpp:402` |

주의: 지형의 픽셀 마스크와 캐릭터의 AABB 판정을 구분한다.
캐릭터 스프라이트 간 알파 픽셀 정밀 충돌이라고 설명하지 않는다.
SweptBox는 이동 구간을 감싼 박스 판정이지 정확한 TOI 기반 연속 충돌 해결이라고 단정하지 않는다.

## 3. 적·보스·소품

| ID | 기능·설계 | 코드에서 확인한 내용 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| E01 | 적 공통 생명주기 | Enemy 가상 함수로 Init/Tick/OnHit/OnDeath/부위 제출, 팩토리 생성 | P1 핵심 / P3 핵심 | `Source/Game/Enemies/Enemy.h:53` |
| E02 | 병사 FSM | Wait/Fall/Land/Idle/Move/Throw/Slash/JumpSlash/Climb 전환 | P1 핵심 | `Source/Game/Enemies/Soldier.cpp:31` |
| E03 | 행동 선택 조건 | 거리, 지붕/지면 상황, 쿨다운, 변형, 확률 선택 | P1 핵심 / P2 핵심 | `Source/Game/Enemies/Soldier.cpp:419`, `:475` |
| E04 | 화면 접근 활성화 | 항상 활성 설정 또는 카메라 우측+마진에 도달한 적 활성화 | P1 보조 | `Source/Game/Enemies/EnemyWorld.cpp:428` |
| E05 | 여러 적 유형 | 병사, 방패, 바주카, 공중/매달림, 헬기, 차량, 버스 등 타입별 구현 | P1 보조 / P2 핵심 | `Source/Game/Enemies/`의 개별 cpp와 Enemy 파생 헤더 |
| E06 | 다부위 보스 | 부위 제출, 부위 피격, 포탑 상태와 취약성, 공격 패턴 전환 | P1 핵심 / P2 핵심 | `Source/Game/Enemies/BigShieeEnemy.h:18`, `:39`, `Source/Game/Enemies/BigShiee.cpp:1196` |
| E07 | 이벤트 연결 | 사망·아이템·해방·변신·제거·부위 파괴를 이벤트로 전달 | P1 보조 | `Source/Game/Enemies/EnemyTypes.h:52` |
| E08 | 파괴 가능 소품 | HP 기반 damaged 단계, 파괴/폭발, 드롭과 파편 | P1 핵심 | `Source/Game/Props/PropWorld.cpp:297`, `:356`, `:411` |
| E09 | 장식 오브젝트 | 클립·속도·뒤집기·스케일과 여러 단계 sequence | P1 보조 | `Source/Game/Props/DecorWorld.cpp:12`, `:143` |

알고리즘 명칭: 상태 머신 + 조건/확률 기반 행동 선택이 확인된다.
Behavior Tree, A*, NavMesh, 머신러닝 기반 AI라는 증거는 확보하지 않았다.

## 4. 스테이지·렌더링·데이터

| ID | 기능·설계 | 코드에서 확인한 내용 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| S01 | JSON 스테이지 정의 | sections/triggerX, Spawn/Wave/Gate/카메라/배경/지형/BGM/종료 액션 | P1 핵심 / P2 핵심 | `Source/Game/Stage/StageData.h:63`, `:227` |
| S02 | 섹션 진행 | 플레이어 X가 트리거를 넘으면 섹션 액션 실행 | P1 핵심 | `Source/Game/Stage/StageRunner.cpp:220` |
| S03 | 지연 실행·게이트 | pending와 gate 상태를 처리하여 진행/스폰을 제어 | P1 보조 | `Source/Game/Stage/StageRunner.cpp:296`, `:386` |
| S04 | 카메라 제어 | bounds·lock·glide·스크롤 및 연출용 이동 | P1 보조 / P2 보조 | `Source/Render/Camera.cpp:111`, `:194` |
| S05 | 2D 화면 범위 판정 | 텍스처와 뷰 사각형 교집합; 범위 밖 텍스처 건너뜀 | P1 핵심 | `Source/Render/SpriteBatch.cpp:65`, `:600` |
| S06 | 큰 텍스처 분할 | strip 텍스처에 걸치는 소스 영역만 선택해 렌더 | P1 보조 | `Source/Render/SpriteBatch.cpp:532` 주변 분할 생성 및 DrawStandalone_ |
| S07 | 화면 뒤 오브젝트 정리 | 카메라 좌측-마진을 지난 소품과 수명이 끝난 파편 정리 | P1 보조 | `Source/Game/Props/PropWorld.cpp:530`, `:597` |
| S08 | 렌더 명령 정렬 | depth → sortKey → 제출 seq로 정렬 | P1 보조 / P2 보조 | `Source/Render/SpriteBatch.cpp:46` |
| S09 | 아틀라스·픽셀 표현 | frame rect/pivot/flip, nearest 텍스처 필터 | P1 보조 | `Source/Render/SpriteBatch.cpp:32`, `:393` |
| S10 | 병렬 이미지 디코드 | 파일·PNG 디코드 병렬화, 요청 결과 순서대로 텍스처 등록 | P3 핵심 | `Source/Render/SpriteBatch.cpp:258`, `:447` |
| S11 | 파일별 데이터 경계 | anim/sprites/sounds/Stage/Defs/Tuning의 역할 분리 | P1 보조 / P2 핵심 | `Source/Render/Animation.cpp:194`, `Source/Audio/SoundBank.cpp:66`, `vcpkg.json` |
| S12 | 튜닝 핫 리로드 | 파일 변경 시간 폴링, 로드 성공 시 값 교체; 실패 시 이전 값 유지 | P2 핵심 | `Source/Core/Tuning.cpp:471`, `Source/Game/SandboxState.cpp:486` |
| S13 | HUD와 화면 흐름 | 별도 HUD/Banner/Fade/BitmapFont/ScreenFlow 모듈 존재 | P1 보조 | `Source/Game/Ui/` |

‘섹터’는 현재 확인된 코드에서 sections 기반 진행·배치 단위이다.
공간 해시·Quadtree·섹터별 모든 객체 업데이트 제거와 동일한 주장으로 쓰지 않는다.
‘프러스텀 컬링’ 대신 **2D 화면 범위 컬링과 보이는 텍스처 영역 계산**을 추천한다.
대용량 처리 성과는 측정값이 필요하다. DecorWorld는 인스턴스를 순회한다.

## 5. 에디터 — 상위 탭 4개, 내부 제작 도구 다수

상위 탭 근거: `Source/Editor/Editor.cpp:225`.

| ID | 추천 이름 | 확인된 기능 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| T01 | 애니메이션 편집기 | 클립 검색, 재생/정지, 프레임 이동, loop/loopStart/frameDuration | P2 핵심 | `Source/Editor/AnimEditor.cpp:244`, `:300` |
| T02 | 피벗 조정 | 프레임별 pivotX/Y, 전체 적용, onion 표시 | P2 핵심 | `Source/Editor/AnimEditor.cpp:339`, `:372` |
| T03 | 애니 편집 이력 | Undo와 저장, 외부 수정 감지 시 미저장 편집 보호 | P2 보조 | `Source/Editor/AnimDocument.cpp:231`, `Source/Editor/AnimEditor.cpp:79` |
| T04 | 스테이지·배치 편집기 | 배치, XY/Z, 복제/삭제, Undo, triggerX와 섹션 이동 | P2 핵심 | `Source/Editor/StageEditor.cpp:927`, `:1016`, `Source/Editor/StageDocument.cpp:761` |
| T05 | 스테이지 보기 | BG/Ground/Grid/Labels/Sprites 레이어와 확대·이동 | P2 보조 | `Source/Editor/StageEditor.cpp:1077` |
| T06 | **지형 충돌 마스크 편집기** | 픽셀 pencil/erase/rect/line/fill, brush, 배경과 mask 겹치기 | P2 핵심 | `Source/Editor/GroundEditor.cpp:694`, `:752`, `Source/Editor/GroundEditor.cpp:326` |
| T07 | 마스크 이력·저장 | PNG 저장, Undo/Redo, 저장 이후 dirty 상태 | P2 보조 | `Source/Editor/GroundEditor.cpp:127`, `:375`, `:389` |
| T08 | 적 AI·전투 튜닝 랩 | 적 선택/배치/제거/재배치, 일시정지·1틱/10틱 진행 | P2 핵심 | `Source/Editor/EnemyLab.cpp:352`, `:422`, `:445` |
| T09 | AI 시각 디버깅 | box/state/target line/ground probe/camera bounds/solids 표시 | P2 핵심 | `Source/Editor/EnemyLab.cpp:385` |
| T10 | 히트박스 편집 | 플레이어·적 크기와 오프셋, 추가 공격/피격 박스, 활성 frames와 damage | P2 핵심 | `Source/Editor/HitboxEditor.cpp:260`, `:356`, `:388` |
| T11 | 공격·모션 튜닝 | 정의의 숫자/벡터/bool, muzzle/fx offset, 발사 프레임, 즉시 발사·모션 재생 | P2 핵심 | `Source/Editor/MotionEditor.cpp:308`, `:402`, `:442` |
| T12 | 깊이 편집 | 부위와 엔티티 Z/depth, 저장 후 정의 재로드 | P2 보조 | `Source/Editor/DepthEditor.cpp:230`, `:294` |
| T13 | 상태 유지 재로드 | 적을 유지하는 ReloadDefsKeep; 불가능한 경우 재생성 fallback | P2 핵심 | `Source/Editor/EnemyLab.cpp:146` |
| T14 | JSON 저장 전략 | Stage/Hitbox/Motion은 원문 숫자 구간 교체; Anim은 Serialize로 새 문자열 구성 | P2 핵심 / P3 보조 | `Source/Editor/StageDocument.cpp:219`, `Source/Editor/HitboxDocument.cpp:129`, `Source/Editor/MotionDocument.cpp:342`, `Source/Editor/AnimDocument.cpp:177` |

명명: ‘픽셀 충돌 프록시’보다 **지형 충돌 마스크 편집기(Ground Collision Mask Editor)**가 정확하다.
실제 PNG의 지형 충돌 데이터를 편집하며, 캐릭터 히트박스 편집기와 별개다.
AI 탭은 ‘적 AI·전투 튜닝 랩’으로 소개하면 행동·모션·박스·스펙의 범위를 담을 수 있다.

## 6. 사운드 — 반드시 영상에 포함

| ID | 기능·설계 | 확인된 내용 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| A01 | 오디오 백엔드 | miniaudio engine/ma_sound와 stb_vorbis 관련 구현 | P1 보조 | `Source/Audio/AudioSystem.cpp:23`, `Source/Audio/StbVorbisImpl.cpp` |
| A02 | 이름 기반 SoundBank | JSON에서 사운드 이름·파일·BGM 구분 읽기 및 인덱스 조회 | P1 핵심 | `Source/Audio/SoundBank.cpp:54`, `:89` |
| A03 | SFX와 BGM 경로 | 효과음 decode 등록, BGM stream/loop, 별도 볼륨 | P1 핵심 | `Source/Audio/AudioSystem.cpp:162`, `:306`, `:372` |
| A04 | 동시 재생 제한 | 기본 16 voices, 같은 사운드 4개 제한, 가장 오래된 voice 교체 | P1 핵심 | `Source/Audio/AudioSystem.h:54`, `Source/Audio/VoiceAllocator.cpp:21` |
| A05 | 피치 변화 | 효과음의 pitchVariation과 별도 AudioRng | P1 보조 | `Source/Audio/AudioSystem.cpp:315`, `Source/Audio/AudioMath.cpp:8` |
| A06 | BGM 상태 전환 | Stopped/Playing/FadingOut; fade 후 pending BGM 재생 | P1 보조 / P3 보조 | `Source/Audio/AudioSystem.h:94`, `Source/Audio/AudioSystem.cpp:239`, `:406` |
| A07 | 장애 대응 | 오디오 장치 초기화 실패 시 무음 계속; 누락 사운드 경고 기록 | P3 별도 | `Source/Audio/AudioSystem.cpp:145`, `Source/Audio/AudioSystem.h:111` |
| A08 | 요청 기록 검증 | bRecord와 events로 요청 종류/순서/매개변수 검사 가능 | P3 핵심 | `Source/Audio/AudioSystem.cpp:268`, `Test/AudioTest.cpp` |

확인되지 않은 주장: 공간 음향/거리 감쇠/복잡한 우선순위 믹서. 현재 Play는 NO_SPATIALIZATION을 사용한다.
음원 제작과 재생 시스템 개발을 구분한다. 음원·스프라이트의 제작자와 출처는 사용자에게 요청한다.

## 7. 검증·리팩터링·AI 활용 후보

| ID | 기능·설계 | 확인된 내용 | 영상·깊이 | 근거 |
|---|---|---|---|---|
| V01 | 모듈별 단위 테스트 | 조작/물리/전투/적/에디터/오디오 등 별도 테스트 파일 | P3 핵심 | `Test/` |
| V02 | 캡처·튜닝 검증 | 7개 상태 캡처와 튜닝 SHA256, 단위 테스트 종료/요약 확인 | P3 핵심 | `Scripts/Verify.ps1:16`, `:47` |
| V03 | 텍스처 로딩 비교 | TraceLoad 스크립트와 baseline 존재 | P3 보조 | `Scripts/TraceLoad.ps1`, `Scripts/Baseline/BaselineTextures.txt` |
| V04 | 외부 SimCheck | 14상태×2입력 패턴×30시점=840 체크포인트를 기준선과 비교 | P3 핵심 | `Tools/SimCheck/Main.cpp:23`, `Tools/SimCheck/Check.ps1:20` |
| V05 | 검증 한계 관리 | 해시에 양자화/필드·RNG 누락 가능성; 코드와 순서 대조 병행 | P3 핵심 | `Docs/ROADMAP.md:56` |
| V06 | 작성자 라이브러리 | JugX 서브모듈 사용. 프로젝트 소스와 사용자 라이브러리 역할 구분 | P3 보조 | `.gitmodules:1`, `Source/Base/Types.h` |

**수치 주의**: 기존 아키텍처 문서에는 테스트 902개, 리플레이 15개 등의 설명이 있다.
현재 ROADMAP의 기록은 895개 테스트와 별도 SimCheck 840개이며 게임 CLI 리플레이는 제거되었다.
이번 조사에서 실행 통과를 재확인하지 않았으므로 영상에는 실제 해당 커밋의 로그를 받은 뒤 수치를 적는다.
hash 일치가 모든 동작의 비트 동일성을 증명하지 않는다.

## 영상용 핵심 필터 결과

- P1 중심: C02~C09, W01~W08, E02/E03/E06/E08, S01/S02/S05, A02~A06.
- P2 중심: T01/T02 → T04/T05 → T06/T07 → T08~T14.
- P3 중심: C01, S10, V01~V06, A08와 사용자 확인을 받은 실제 문제 해결 사례.
- 후속 문서/영상으로 넘길 후보: 상세 팩토리/소유권, 음원 로딩 실패 대응, 모든 적 패턴, 모든 튜닝 필드.
- 자료 부족: 개발 연도별 기여, 실제 성능 개선 수치, 에디터 제작 전후 작업 시간, AI 제안 거절 사례, Source1 버그의 정체.

## 기술 표기 초안

C++20 / SDL3 / Dear ImGui / yyjson / miniaudio / stb / JugX.
검증 도구: GoogleTest / PowerShell.
manifest에는 parallel-hashmap과 utfcpp도 존재한다. 의존성으로 등록된 것과 현재 직접 사용한 것은 구분한다.
Claude는 런타임 기술 스택이 아니라 2026 개발·리팩터링에 사용한 도구로 별도 표기한다.

