# 데이터, 스테이지 실행, 편집기, 오디오 읽기

이 문서는 `0ef7104`의 코드를 읽어서 정리한 학습 자료다. Source 변경, 빌드, 테스트, 게임 실행은 하지 않았다. 코드는 현재 동작의 근거이고, 작성 주체·개발 연도·개인의 시행착오를 증명하는 근거는 아니다. 아래의 **코드로 확인**은 실제 분기와 호출을 뜻하고, **설계 해석**은 그 구조를 이해하기 위한 설명이다.

전체 흐름은 파일의 문자열을 게임이 소유하는 값으로 읽고, 그 값을 스테이지 실행기가 해석하여 월드에 명령을 보내는 것이다. 편집기는 같은 파일을 사람이 조작할 수 있게 보여준다. 저장된 파일과 실행 중인 객체는 서로 다른 상태이므로, 저장과 즉시 반영을 구별해서 읽어야 한다.

## 1. Assets를 먼저 읽으면 코드의 목적이 보인다

| 파일 | 담는 값 | 실제 읽는 쪽 | 구별할 점 |
|---|---|---|---|
| `Assets/manifest.json` | atlasPages, animations, sprites, sounds, ground 이름과 경로 | AnimationLibrary, 에셋 탐색 | 모든 로더가 manifest의 각 경로를 사용하는 것은 아니다 |
| `Assets/anim.json` | 이름별 애니메이션, 프레임의 atlas 사각형·피벗, 반복·속도 | AnimationLibrary, AnimDocument | 런타임 읽기와 편집용 읽기의 허용 필드가 다르다 |
| `Assets/sprites.json` | atlas sprite 또는 독립 이미지 file/w/h | SpriteLibrary, Map preview | 큰 배경은 독립 텍스처로도 읽는다 |
| `Assets/sounds.json` | 사운드 이름→상대 파일, bgm 여부 | SoundBank | 버전 필드가 있다고 모든 로더가 버전 검사를 하는 것은 아니다 |
| `Assets/Tuning/Tuning.json` | 이동·몸체·총구·무기·칼·폭탄·플레이어·카메라·샌드박스·아이템·리그 | TuningReadVisitor | 빠진 값은 C++ 기본값; 임의 키 거부 방식은 아니다 |
| `Assets/Defs/*.json` | 적·보스·적 투사체의 정의, 상속, 행동별 params | EnemyDefLibrary | 폴더의 최상위 JSON만 모아 읽는다 |
| `Assets/Defs/Props/Props.json` | 소품 정의, 아이템, 랜덤 아이템 그룹 | PropDefLibrary | 적 정의의 extends 상속과 별도 형식 |
| `Assets/Stage/*.json` | 월드·시작점·카메라·섹션·액션 | StageParser, StageDocument | 런타임 스키마 검사와 편집용 원문 색인은 별개 |
| `Assets/Ground/*.png` | 픽셀 지면 마스크 | LoadGroundMask, GroundMaskDocument | 화면에 보이는 배경 그림과 실제 충돌 마스크를 분리 |
| `Assets/Atlas/*.png`, `Assets/Textures/*.png` | 실제 그림 픽셀 | 런타임 렌더러, 편집기 preview | JSON이 가리키는 사각형·피벗과 함께 사용 |
| `Assets/Audio/*` | wav/mp3/ogg 음원 | miniaudio, stb_vorbis | 이름 문자열과 파일 경로를 SoundBank가 연결 |

근거: [manifest](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Assets/manifest.json:1>), [anim 표본](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Assets/anim.json:4>), [sound 목록](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Assets/sounds.json:3>).

### 1.1 경로를 찾는 방식

`FindAssetRoot()`는 실행 파일 폴더에서 시작하여 자신과 최대 다섯 부모 단계의 `Assets`를 찾는다. `manifest.json` 또는 `Tuning/Tuning.json`이 있으면 에셋 폴더로 인정한다. 못 찾으면 설정한 fallback, 마지막으로 현재 작업 폴더의 Assets를 쓴다. 파일 읽기는 JugX FileReader로 크기만큼 읽고 실제 읽은 길이를 확인한다. 저장도 JugX FileWriter로 직접 쓰며, 임시 파일을 통한 교체 방식은 아니다.

근거: [AssetPath.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:71>), [파일 저장](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:113>).

### 1.2 그림 데이터의 두 형식

atlas형 프레임은 `atlas/x/y/w/h/pivotX/pivotY`를 갖는다. x/y/w/h는 atlas에서 잘라낼 사각형이고 pivot은 그 그림을 게임 좌표에 붙이는 기준점이다. 피벗이 달라지면 몸의 논리 위치가 같아도 그림이 달리 정렬된다.

독립 sprite는 `file/w/h`를 갖는다. SpriteLibrary는 `file`이 있으면 독립 이미지, 없으면 atlas형으로 분기한다. 이름이 이미 있으면 기존 슬롯을 교체하고, 새 이름이면 벡터 뒤에 넣는다. AnimationLibrary는 manifest에서 애니메이션 파일명과 atlasPages를 읽는다. SpriteLibrary와 SoundBank의 directory loader는 각각 `sprites.json`, `sounds.json`을 직접 읽는다.

근거: [SpriteLibrary 파싱](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:9>), [Animation manifest 읽기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:194>), [SoundBank 파일 읽기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:66>).

## 2. 로드와 검증은 데이터 종류마다 다르다

JSON 파싱은 문자열의 문법을 읽는 단계다. 스키마 검사는 필드의 이름·타입·범위를 확인하는 단계다. 참조 검사는 문자열이 가리키는 다른 정의나 ID가 실제로 존재하는지를 확인하는 단계다. 같은 JSON을 쓴다고 세 단계가 모든 로더에 똑같이 들어 있지는 않다.

| 경계 | BOM | 주석 | 주요 정책 |
|---|---|---|---|
| 적 정의, 소품 정의, Tuning | 허용 | 허용 | 각 로더의 필드 규칙 사용 |
| Stage, Animation, Sprite, Sound | 허용 | comments 옵션 없음 | 각 파일 형식의 필수 필드 검사 |
| StageDocument 초기 Load | 정식 JSON 읽기로 먼저 검사 | comments 옵션 없음 | 이후 원문 scanner로 편집 색인 구성 |
| HitboxDocument | scanner가 BOM 건너뜀 | scanner는 공백만 건너뜀 | 원문 수집 규칙; 전체 적 정의 검증을 하지 않음 |
| MotionDocument 초기 LoadFromText | 원문 그대로 | 원문 그대로 | 문자열 보관만 함; 수정할 때 scanner로 경로 찾음 |

일반 JSON 정수 읽기의 일부는 숫자를 `llround`한다. 반면 StageParser의 정수 필드는 소수부가 있거나 절댓값이 2,147,483,647보다 크면 거부한다. 따라서 모든 숫자 필드가 같은 반올림 규칙이라고 설명하면 틀린다.

근거: [JsonFields::ReadInt](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:48>), [StageParser::GetInt_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:644>).

### 2.1 적 정의의 상속

EnemyDefLibrary의 폴더 로더는 최상위 `*.json` 파일을 경로순으로 정렬한다. JsonDeserializer들을 벡터에 보존한 채 reader를 모으고, 모든 root의 raw def를 먼저 수집한다. 이 때문에 다른 파일에 있는 부모와 투사체도 함께 참조할 수 있다. 중복 정의 이름과 지원하지 않는 version을 거부한다. globals는 읽은 순서에 따라 값이 갱신된다.

`BuildChain_()`은 extends 부모를 재귀로 찾고 **부모부터 자식까지** reader를 넣는다. 깊이가 8을 넘으면 너무 깊거나 순환한다고 오류를 낸다. 별도의 방문 집합으로 정확한 순환 경로를 추적하는 구조는 아니다. `abstract`인 raw 정의는 실제 런타임 적·투사체로 생성하지 않는다.

단일 필드는 자식부터 역순으로 찾아 마지막 정의를 사용한다. `clips/weapons/params` 같은 묶음은 부모부터 key를 합치고, 자식의 같은 이름 항목이 기존 항목을 교체한다. 이것은 JSON 전체를 임의 깊이까지 재귀 병합하는 규칙과 다르다. 어떤 객체는 가장 가까운 객체 전체가 선택된다.

파싱은 두 차례 진행한다. 투사체를 먼저 넣고 적을 다음에 넣어, 적 무기가 투사체 index를 찾을 수 있게 한다. kind/behavior/team의 알려진 값, 필수 hp/body, hp≥1, 상자 크기, clip·weapon 참조 등을 검사한다. `ttk × dpsRef`와 hp가 5% 이상 다른 것은 경고이며 hp를 자동으로 계산해 덮어쓰지 않는다.

근거: [폴더 수집](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1467>), [상속 chain](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:419>), [Find/Merge](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:468>), [투사체 우선 파싱](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1345>).

행동별 params는 `DefReader`를 통해 Config로 변환한다. 읽은 키는 사용했다고 표시하고 Finish에서 사용되지 않은 키를 오류로 처리한다. Soldier/Bzk/Hang/FlyingBzk/Shielder/Pow/Jeep/BTank/MissileTank/Sarubia/Bus/Heli/HeliBomb/BigShiee의 14개 분기가 있다. 필요한 논리 clip/weapon이 빠져도 오류다. `params.parts`는 해당 행동에 존재하는 부품명과 `depth/sort` 정수만 허용한다. 마지막으로 FlyingBzk의 landedDef, Bus의 병사 정의가 존재하는지 확인한다.

근거: [행동 Config 선택](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1230>), [사용하지 않은 params 검사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:397>), [parts 검사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:496>).

**설계 해석:** 게임 안에서는 문자열 검색과 상속 해석을 반복하기보다, 로드 때 행동별 Config와 index를 만들어 실제 적이 사용할 값을 정리하는 구조다. 상속은 중복 데이터 작성량을 줄이지만, 편집기는 원문에 직접 쓰인 값과 상속된 결과를 구분해야 한다.

### 2.2 소품과 아이템

Props.json의 root는 defs/items/groups다. 소품에는 상태별 시각 자료, hp, depth, 여러 parts, 파괴 시 폭발·파편·drop·자식 spawn 등이 들어간다. 아이템은 gun/supply/score를 구분하고, groups는 아이템 이름 배열이다. drop은 아이템 또는 그룹, 자식 spawn은 소품 정의를 참조하며 끝에서 존재 여부를 검사한다. 적 정의의 extends 처리와 같은 상속 체계로 설명하지 않는다.

스테이지의 prop params는 hp/sprites/hitbox/standBox/explodeOnDeath/variant를 받는 별도 override다. `MakePropSpawnParams()`는 ParamValue의 값을 실제 PropSpawnParams로 옮긴다. 같은 이름의 params라도 적의 행동 Config와 이 prop override는 서로 다른 용도다.

근거: [Props root 및 참조 검사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:723>), [prop params 검사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:832>), [stage mapping](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:46>).

### 2.3 ParamValue가 필요한 이유

StageData는 JugX JSON reader를 계속 보유하지 않는다. params의 bool/number/string/array/object를 `ParamValue`로 재귀 복사한다. 문자열과 하위 항목을 자체 소유하므로 원래 JsonDeserializer가 끝나도 스테이지 데이터가 유효하다. Object는 keys와 items의 같은 인덱스를 대응시키고 Set은 기존 키를 교체하거나 새 키를 뒤에 추가한다. AsInt는 llround, 잘못된 타입은 기본값, 없는 array element는 정적 NullValue를 돌려준다. TryGetVec2는 정확히 숫자 2개를 요구한다.

근거: [ParamValue 선언](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:3>), [소유하는 값으로 복사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:377>), [형 변환과 접근](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:92>).

## 3. StageData는 장면의 순서와 조건을 기술한다

StageData의 root는 version/name/world/bgm/ground/backdrop/player/rects/camera/sections다. version은 1이다. world size와 view는 양수, ground initial은 maps에 존재해야 하고 backdrop initial은 modes에 존재해야 한다. rect는 `[left, top, right, bottom]`이며 right>left, bottom>top이다. `current`는 예약된 rect 이름이며 camera lock의 현재 화면을 뜻할 때 허용된다.

현재 Stage0는 14,094×1,328 월드, 640×448 view, Field_BGM, Ground Stage0_0/Stage0_1, 초기 camera bounds `[0,0,3520,448]`, playerMargin 14를 가진다. 섹션은 intro→street→ground_swap→bus→boardwalk→bridge→bridge_down→bridge_mid→bridge_end→river_start→heli_river→river_mid→river_end→downtown_start→downtown_mid→downtown_end→openland_start→openland_1→openland_2→openland_3→openland_4→boss_arena의 22개다. 이는 현재 데이터의 목록이며 녹화 버전이 반드시 동일하다는 뜻은 아니다.

근거: [StageData 형식](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:295>), [root 파싱](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:773>), [현재 Stage0](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Assets/Stage/Stage0.json:1>).

### 3.1 spawn의 좌표와 선택값

kind는 enemy/npc/prop/decor/item/collider다. enemy와 npc의 기본 origin은 feet, 나머지는 legacyCenter다. enemy/npc를 legacyCenter로 쓰면 halfHeight를 주어 발 위치로 변환한다. 다른 종류의 legacyCenter는 중심 좌표로 전달될 수 있다.

직접 x/y 또는 camera anchor 중 하나를 선택한다. camera와 x/y, camera와 groundSnap은 함께 쓸 수 없다. groundSnap이면 y를 생략하고 fromY부터 바닥을 찾으며 origin은 feet여야 한다. repeat는 count≥1, stepX/stepY의 공간 반복이고, wave의 every/times는 시간 반복이다. 둘을 혼동하면 배치 수와 생성 시점이 달라진다.

추가 값으로 facing, z, hp, drop, ambush 범위, layer, cullMargin, settle, params가 있다. hp override는 양수, cullMargin은 음수가 아니어야 한다. size/collisionLayer는 collider 전용이다. decor는 anim/sprite를 사용하고 다른 kind에 그 필드를 붙이지 않는다. camera dx는 정수 또는 `[min,max]` 정수 범위로, 범위가 실제로 다를 때 spawn마다 RNG를 소비한다.

근거: [SpawnSpec](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:129>), [좌표 검증](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1237>).

스테이지 parser는 section ID 중복, triggerX 역순, spawn ID 중복, gate ID 중복, gate waitFor/command target의 알 수 없는 spawn ID를 거부한다. 섹션은 하나 이상 필요하다. boundsCurve의 from/until은 존재하고 from이 먼저여야 한다. 다만 이 검사는 모든 def 문자열이 실제 적/소품 라이브러리에 존재함을 보장하는 범용 검사와 같지 않다. 실제 spawn 실패도 런타임이 처리한다.

근거: [섹션 순서](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1783>), [ID 참조 검사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1945>).

### 3.2 17개 액션의 실제 실행

| 액션 | 값과 역할 | 실행 경계 |
|---|---|---|
| spawn | SpawnSpec 하나 | host SpawnEntity |
| spawnWave | entries의 delay/every/times와 SpawnSpec | 예약한 tick에 생성 |
| setBounds | rect | permanent lock이 있으면 무시 |
| lockCamera | rect/current, glide, permanent | lock stack에 추가 |
| unlockCamera | 별도 값 없음 | 뒤에서 가장 최근 nonpermanent lock만 제거 |
| cameraMove | to, seconds | silent면 seconds=0 |
| setCameraOffset | offset | 이후 camera target에 더함 |
| setBackdrop | mode | 상태 기록+host 호출 |
| setGround | name | 상태 기록+host 호출 |
| gate | id, waitFor, until, actions, abortOnGameOver | 조건 충족 시 내부 actions |
| command | target, command | ID 기록의 모든 SpawnId에 host SendCommand |
| clear | waves, layers | 예약 wave/gate 정리 또는 layer entity 정리 |
| banner | text, seconds, optional sfx | banner 후 sfx |
| fade | black/clear, seconds | host FadeTo |
| playBgm | name, loop, fadeOutSec | fadeOutSec>0이면 ChangeBgm |
| playerInvincible | seconds | host에 무적 시간 전달 |
| endStage | optional next | finished=true, host EndStage |

근거: [enum과 공통 액션값](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:63>), [Execute_ 전체 분기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:454>).

**기능 한계:** command는 parser·runner·테스트 계약에 있는 액션이지만 현재 StageWorldHost의 적 명령 분기는 반환만 한다. 다른 대상은 한 번 경고한다. 실제 적 AI를 문자열 명령으로 제어하는 완성 기능처럼 소개하지 않는다. [SendCommand](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:557>).

### 3.3 한 tick의 실행 순서

StageRunner는 StageData/IStageHost/Camera/RNG를 빌려 쓰는 포인터를 갖고, 예약 항목·gate·spawn 기록·lock stack·현재 ground/backdrop을 자체 소유한다. 예약 항목은 StageData 내부 액션/entry 포인터를 갖기 때문에 데이터의 수명과 위치가 실행기보다 오래 유지되어야 한다.

Begin은 tick/section/예약/gate/기록/lock을 초기화하고 camera policy·bounds·offset, ground/backdrop/BGM을 적용한다. Tick의 순서는 다음과 같다.

1. game over이면 abortOnGameOver gate/예약을 제거한다.
2. player feet.x가 다음 triggerX보다 **엄격히 크면** 섹션을 실행한다. 같은 tick에 여러 섹션을 넘을 수 있다.
3. dueTick≤현재 tick인 예약을 모아 dueTick, seq 순으로 안정 정렬하여 실행한다.
4. 활성 boundsCurve를 playerX로 계산한다.
5. gate 조건을 확인하고 만족한 gate를 제거한 뒤 내부 액션을 실행한다.
6. player feet 기준 target을 설정하고 camera Update(고정 step)한다.
7. tick을 하나 올린다.

근거: [Begin](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:52>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:210>), [예약 정렬](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:265>).

seconds는 `llround(double(seconds)×60)`으로 tick이 된다. 시간 반복의 다음 dueTick은 현재 실행 tick이 아니라 이전 dueTick+everyTicks다. 이번 ProcessPending에서 꺼낸 ready 배열과 새로 넣는 pending은 별개이므로 재등록된 항목은 다음 처리에서 본다. clear waves는 pending wave를 지우고 모든 gate를 비우며 cutoff seq를 기록한다. 이미 ready로 꺼낸 오래된 wave도 cutoff 검사로 막는다.

gate는 waitFor의 각 문자열 ID에 대응하는 **모든 실제 SpawnId**를 검사한다. ID 기록 자체가 없으면 아직 조건이 충족되지 않는다. 기록은 있지만 spawn이 모두 실패해 ID 배열이 비면 검사할 항목이 없어 만족한다. 이 둘의 차이를 알아야 groundSnap 실패 뒤 gate 동작을 설명할 수 있다.

근거: [시간 환산](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:46>), [gate 충족](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:349>), [clear](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:611>), [spawn 기록](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:759>).

### 3.4 카메라, warp, ground mapping

camera target은 `(playerFeet.x+offset.x, playerFeet.y−60+offset.y)`다. base bounds 위에 가장 최근 lock rect가 유효 범위가 된다. Unlock은 permanent lock을 건너뛰며, permanent가 하나라도 있으면 setBounds와 curve 갱신을 막는다. boundsCurve는 점 사이를 선형 보간하고 선택에 따라 trunc한다. 여러 활성 curve가 있으면 코드 순서상 뒤의 값이 base를 덮어쓴다.

forward policy에서는 화면 왼쪽 벽을 만든다. lock/move/pending-lock 또는 오른쪽 bounds 끝에 도달하면 오른쪽 벽도 만든다. view rect는 camera position+camera viewSize다. AddCameraScroll은 lock 중이면 마지막 lock rect를 옮기고, 아니면 camera scroll floor를 올린다.

Warp는 뒤로 가거나 finished일 때 거부한다. pending/gate/nonpermanent lock을 정리하고 지나친 섹션을 silent로 재생한 뒤 player x·ground y를 바꾸고 camera를 snap한다. when은 normal/silent/always다. 기본적으로 enemy spawn은 normal, prop/decor/item/collider/npc spawn과 setBounds/setBackdrop/setCameraOffset/unlockCamera는 always다. 모든 액션이 silent로 재생되는 것은 아니다. target section은 warp 중 직접 Fire하지 않고 이후 x>trigger Tick에서 처리된다.

Ground mask는 RGBA의 R<80, G<80, B≥200, A≥128인 픽셀을 solid로 만든다. collider는 feet이면 bottom-center, legacyCenter이면 center-size, 누락/비양수 size는 16×16으로 보정한다. item center를 feet로 바꿀 때 y+16을 사용한다.

근거: [camera target/벽](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:182>), [warp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:831>), [DefaultWhen_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:353>), [StageMapping](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:91>).

**설계 해석:** StageParser는 파일 규칙과 참조를 담당하고 StageRunner는 시간·조건·순서를 담당하며 IStageHost는 실제 월드 작업을 담당한다. runner가 ImGui나 구체적인 적 객체를 직접 알아야 할 이유가 줄어든다. 현재 구조의 책임 설명이며 유일한 최적 설계라고 주장하는 것은 아니다.

## 4. Tuning은 같은 필드 목록으로 읽기와 덤프를 공유한다

Tuning.h의 기본값은 실행 가능한 기본 설정이다. 대표적으로 run/air speed 250, crouch 125, jump 600, gravity 1400, terminal 1000, coyote/jump buffer 4tick, shot/bomb buffer 6tick이다. WeaponTuning은 일반 탄뿐 아니라 HMG burst, 로켓 blast/가속, 화염 particle 값을 함께 갖는다. Knife/Bomb/Player/Camera/Sandbox/Item/Rig는 해당 책임별 묶음이다. 값의 단위는 이름의 Ticks, Sec, Px 또는 설명을 확인한다.

`Visit_`가 move/body/muzzle/weapons/knife/bomb/player/camera/sandbox/items/rig의 필드를 한 번 나열한다. TuningReadVisitor는 JSON object stack으로 읽고 DumpVisitor는 같은 방문 순서로 `path = value` 텍스트를 만든다. Dump는 JSON 저장 기능이 아니다. 없는 object/field는 기본값을 유지하고, 존재하지만 타입이 잘못된 값은 전체 parse를 실패시킨다. float/int/bool/vector2 overload가 있고 int는 llround, vector2는 정확히 숫자 두 개다. 알 수 없는 키를 방문하지 않으므로 적의 DefReader::Finish와 정책이 다르다.

근거: [기본값](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:17>), [공통 방문](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:66>), [읽기 visitor](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:206>), [덤프 visitor](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:336>).

TuningWatcher는 기본 60번 Poll 호출마다 mtime을 확인한다. 바뀌면 timestamp를 먼저 기록하고 새 Tuning을 로드한다. 실패하면 이전 값을 유지한다. 같은 잘못된 파일을 매번 다시 읽지 않고 다음 mtime 변화까지 기다린다. 성공하면 전체 값을 교체하고 reload count를 늘린다.

**실제 적용 범위:** SandboxState는 처음 Tuning을 성공적으로 읽었을 때 watcher를 켜고, Tick에서 Poll이 성공하면 camera tuning과 player rig binding을 갱신한다. StageState는 초기 LoadTuning 경로는 있지만 같은 watcher polling은 없다. '모든 실제 스테이지의 모든 값이 자동 리로드된다'고 확장하지 않는다.

근거: [watcher](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:452>), [Sandbox 초기 감시](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:47>), [Sandbox 다음 tick 적용](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:466>), [Stage 초기 로드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:207>).

## 5. 에디터의 네 탭과 문서 소유권

Editor는 Animation/Map/Collision/AI의 네 탭을 제공한다. AI 안에는 EnemyLab, Hitbox, Motion, Depth 창이 있다. 따라서 '일곱 개 탭'보다는 '네 탭, AI 안의 여러 편집 창'이 정확하다. Editor가 UI와 문서 모델을 값으로 소유하고 AnimationLibrary·Renderer·SimRunner는 필요한 포인터로 연결한다.

ImGui는 SDL3 이벤트를 받고 SDLRenderer3로 그린다. docking과 detached viewport가 켜져 있다. 보조창은 같은 renderer의 texture target에 UI를 그린 뒤 픽셀을 창 surface에 복사한다. 매 Editor frame에는 게임 논리 해상도 프레젠테이션을 잠시 끄고 UI를 그린 뒤 복구한다.

keyboard 소유권은 WantCaptureKeyboard/WantTextInput 또는 dock host가 아닌 navigation window focus로 판단한다. App은 에디터가 새로 키보드를 소유하면 게임 입력을 비우고 키다운을 게임으로 보내지 않는다. 마우스도 Lab은 WantCaptureMouse와 별도 captured flag를 확인한다. 이 경계를 읽어야 숫자 편집 중 캐릭터가 같이 움직이지 않도록 하는 의도를 설명할 수 있다.

근거: [Editor 탭](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:218>), [frame/keyboard](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:124>), [App 입력 경계](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:88>), [보조 viewport](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:57>).

### 5.1 Animation: 구조화된 값과 clip snapshot

AnimDocument는 전체 클립과 프레임을 구조화된 값으로 소유한다. 원문을 보존하는 방식이 아니므로 root/clip/frame의 기대 필드 수를 확인해 모르는 필드가 저장 과정에서 사라지는 상황을 거부한다. Save는 version/animations 전체를 새 JSON으로 직렬화한다. undo는 바뀌기 전 클립 하나의 snapshot이며 최대 200개다. Undo 후 dirty=true가 되고 revision이 증가한다. redo는 없다.

AnimEditor는 anim.json과 atlas_NNN.png를 읽고 선택·frame·재생시간·onion·zoom·SDL texture cache를 갖는다. clip의 pivot/duration/loop/loopStart를 바꾸면 revision이 바뀐다. Editor::Frame의 Poll에서 revision을 비교하여 바뀐 clip 하나 또는 전체를 런타임 AnimationLibrary에 Apply한다. Save 전에 preview에 반영될 수 있다.

60 editor frame마다 disk mtime도 확인한다. 미저장 편집이 있으면 원문 변경을 자동 덮어쓰지 않고 경고한다. clean이면 Reload하여 선택 이름을 복구하고 library에 적용한다. Save 성공 뒤에만 ClearDirty와 mtime 갱신을 한다. ApplyClipTo는 비어 있는 frame/비양수 duration을 건너뛰고 loopStart를 clamp한다. optional cancelFrame은 문서 직렬화에서 보존하지만 이 runtime Apply 함수가 구성하는 AnimClip에는 복사하지 않는다.

근거: [AnimDocument load/save/undo](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:126>), [revision sync/mtime](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:57>), [runtime apply](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:574>).

### 5.2 Map: 원문 span patch와 전체 텍스트 undo

StageDocument는 raw text/saved text와 entity/section/rect의 위치 색인을 소유한다. JsonScanner의 begin/end는 원문 UTF-8 문자열의 byte offset이다. 뒤쪽 replacement부터 적용하므로 앞부분의 위치가 아직 유효하다. 변경 뒤 Reparse로 색인을 재생성하고 실패하면 원문과 색인을 복원한다. undo는 전체 텍스트 snapshot 최대 100개다.

수집 대상은 player, spawn, spawnWave entries, gate 내부 spawn이다. lockCamera/setBounds rect와 section trigger도 찾는다. SetPosition은 존재하는 x/y만 바꾸고, groundSnap의 y는 별도 UnlockGround로 고정값을 넣는다. SetZ는 override를 추가/교체하거나 기본값이면 지울 수 있다. Duplicate는 기존 object 원문을 복사하고 좌표·ID만 바꾸며 ID에 _2.._999 후보를 붙인다. Delete는 주변 쉼표까지 조정한다. player는 복제·삭제 대상이 아니다.

Map은 배경·마스크·sprite·grid·labels와 section의 trigger/rect를 함께 보여준다. 스프라이트 preview는 애니메이션의 첫 프레임을 사용하고 {side} 그림이 없으면 반대 방향을 mirror한다. Ctrl+S/Z/D, Delete, Esc가 있고 wheel zoom(0.05~4), Shift wheel 수평 이동, middle/right drag pan을 지원한다. 배치 좌표는 기본 1px, Shift일 때 8px snap이다. 드래그 첫 변화에만 undo를 남기고 이후 같은 드래그는 추가 snapshot을 만들지 않는다.

**반영 경계:** Save는 Stage JSON에 쓰는 것까지다. StageEditor::Save_에 실행 중인 StageRunner 재생성이나 자동 stage reload 호출은 없다. 문서를 읽었다고 런타임 스키마·참조를 전부 검사한 것도 아니다. StageDocument의 editor index와 StageParser의 game validation은 분리되어 있다.

근거: [원문 추출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:87>), [내림차순 patch](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:219>), [Map save](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:723>), [mouse editing](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:1304>).

### 5.3 Collision: 픽셀 stroke와 Undo/Redo

GroundMaskDocument는 픽셀마다 0/1을 갖고, stroke마다 index/before/after를 기록한다. 연속 pencil drag 전체가 하나의 stroke다. 선은 정수 오차를 갱신하며 Stamp를 이어 붙이고, 사각형은 양 끝을 포함한다. FloodFill은 같은 색의 네 방향 이웃을 스택으로 방문한다. Undo/Redo는 history의 적용 위치를 앞뒤로 움직이고 before/after를 다시 쓴다. 저장 위치 이후를 되돌린 뒤 새로 그려 redo branch를 잘라내면 savedLost로 dirty 상태를 유지한다.

그림은 blue solid/white air의 8bit palette PNG로 저장한다. 압축은 stb zlib, PNG chunk CRC도 작성한다. SDL preview는 폭 2048 strip으로 나누고 바뀐 열 범위만 업로드한다. 모든 frame에 전체 마스크 texture를 다시 만드는 방식은 아니다.

Pencil/Eraser/Rect/Line/Fill 도구와 B/E/R/L/G 단축키, Ctrl+S/Z/Y가 있다. left click은 그리기, right click은 지우기이고 Shift로 마지막 점부터 선을 잇는다. zoom은 0.125~16, 픽셀 grid는 zoom≥8에서 표시한다. 미저장 상태에서는 다른 mask 선택을 막는다. Save는 PNG를 쓰며 실행 중인 월드 지면을 직접 재로드하는 호출은 없다.

근거: [stroke와 history](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:206>), [palette PNG](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:57>), [dirty column upload](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:588>), [canvas 입력](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:904>).

### 5.4 AI: 재현하는 Lab과 그 안의 세 편집 창

EnemyLab은 SimRunner를 통해 enemylab 상태를 열고 이전 상태로 돌아간다. SandboxState 포인터는 dynamic_cast로 얻는다. 적을 생성하면 정의명/발좌표/facing/z를 remembered set에 넣는다. Clear는 enemy reset(seed 1)과 projectile clear, RespawnLast는 set을 다시 생성한다. Pause/Step/시간배율은 App의 실제 fixed-step 수에 연결된다.

Lab은 적의 body/hurt/attack 상태·target·probe·camera·solids를 그린다. 월드 클릭은 SDL window 좌표→logical 화면 좌표→floor(camera)+world 좌표로 바꾸고 적을 배치하거나 선택한다. Defs 최상위 JSON의 최대 mtime을 60 render frame마다 비교한다. Motion에 미저장 변경이 있으면 자동 reload를 막는다. ReloadDefs는 새 library+set respawn, ReloadDefsKeep는 기존 적 유지 방식이 실패할 때 전체 reload/respawn으로 돌아간다. Props 하위 폴더는 이 watcher 대상이 아니다.

근거: [spawn/reload](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:72>), [mtime watcher](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:176>), [step](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:268>), [world mouse](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:517>).

HitboxDocument는 W/H/Cx/Cy 숫자의 span을 모은다. 일반 w/h 객체, *Size/hurt* 숫자쌍과 대응 *Offset을 찾고, 추가 hitboxes 배열은 별도로 처리한다. SetValue는 해당 숫자 token만 교체한다. Revert는 마지막 저장 원문으로 돌아가며 단계별 undo는 없다. Extra hitboxes는 hurt/attack, offset, frames, damage를 편집하고 배열이 비면 해당 필드를 지울 수 있다.

HitboxEditor의 직접 live 반영은 player stand/crouch/hurt 크기 네 종류, enemy body/hurtbox, BigShiee cannonBoxSize/Offset이다. enemy body 크기는 기존 body RefreshBodies도 호출한다. 화면에 나열된 임의 숫자 전체가 즉시 runtime에 반영된다고 말하지 않는다. Save/Revert는 Lab defs reload 경로를 사용한다.

근거: [Hitbox 원문 patch](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:127>), [정확한 live 대상](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:117>), [Save/Revert](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:318>).

MotionDocument는 무기의 muzzle/muzzleFx offset/motionClip/effects, clip sec, part depth/sort, params를 원문 경로로 바꾼다. 숫자는 소수 네 자리로 round하고 epsilon 0.00005 이상 차이에서 token을 교체한다. ListParams는 해당 정의에 직접 쓴 number/bool/vector2와 최대 세 단계의 하위 object를 목록으로 만든다. 상속 결과 전체를 다시 child 원문으로 써주는 범용 serializer가 아니다.

MotionEditor는 runtime EnemyDef를 바꾸어 발사/모션과 marker drag를 확인한다. 무기·clip 변경은 changed 목록에 기록하고 Save 버튼에서 원문에 쓴다. 원문에 없는 상속 항목은 skipped로 알려준다. params는 변경 즉시 SetParam→Save→ReloadDefsKeep를 수행한다. 다른 미저장 변경이 있으면 이를 먼저 Save한다. 따라서 'Motion은 모두 저장 버튼을 누를 때 반영'과 'Motion은 모두 자동 저장' 모두 부정확하다.

DepthEditor는 compiled depth 상수를 보기용으로, 여섯 정의 파일의 depth/fxDepth를 편집용으로 보여준다. part depth/sort는 runtime 값을 바꾸고 MotionDocument::SetPartDepth로 nondefault override를 추가한다. 일반 row는 숫자 span만 바꾸고 저장 뒤 full reload/respawn한다. 현재 Save는 WriteFileBytes 결과를 버리고 dirty=false로 처리하므로 저장 버튼의 상태 문구만으로 성공을 증명할 수 없다.

근거: [Motion patch](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:84>), [Motion 저장](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:137>), [params 즉시 저장](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:339>), [Depth 저장 경계](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:289>).

### 5.5 실제 편집을 따라가는 세 시나리오

**Map에서 물체를 한 번 끌기:** canvas에서 물체 marker를 클릭→선택과 grab offset 기록→mouse world 좌표를 snap→첫 변경에서 SetPosition(undo=true)→뒤쪽 token부터 원문 교체→Reparse→이후 drag 변경은 undo=false→Ctrl+Z이면 drag 시작 전 원문으로 복원→Ctrl+S이면 파일 저장. 이 과정에서 실행 중인 스테이지의 spawn 목록을 즉시 교체하는 단계는 없다.

**Animation 피벗 이동:** 방향키 또는 숫자 입력→SetPivot이 이전 clip snapshot 기록→frame pivot 변경+revision 증가→그 Editor frame의 Poll에서 ApplyClipTo→다음 game render가 library의 새 frame 값을 사용할 수 있음. 게임 simulation tick과 editor render frame이 같은 것은 아니며, 일시정지 중에도 editor preview/sync 경로는 진행될 수 있다.

**AI params 변경:** 해당 정의를 선택→직접 쓰인 params 숫자 drag→기존 미저장 weapon 변경이 있으면 먼저 Save→숫자 token patch→파일 쓰기→ReloadDefsKeep→다음 적 Tick에서 새 정의값을 읽음. 기존 적을 유지하는 조건이 실패하면 set respawn으로 바뀔 수 있다. 따라서 영상에서 값을 바꾸는 장면만으로 '같은 적이 동일 상태에서 그대로 반영'했다고 단정하지 않는다.

**설계 해석:** 문서 모델은 저장 가능한 값 또는 원문과 편집 이력을 책임지고, UI는 마우스·선택·확대·텍스처를 책임진다. 구조화된 애니메이션은 clip 단위 undo가 자연스럽고, 임의 필드가 섞인 Stage/Defs JSON은 원문 span patch가 불필요한 재포맷을 줄인다. PNG는 문자가 아니라 픽셀 변경 단위가 알맞다. 이는 현재 구현을 읽은 설명이며 사용자 스타일이 확정되었다는 뜻은 아니다.

## 6. 오디오는 이름, voice 선택, 장치 backend를 구분한다

SoundBank는 sounds object의 이름/file과 optional bool bgm을 읽는다. file은 비어 있으면 안 되고 assetRoot/file을 실제 경로로 만든다. name→index map과 SoundEntry 벡터를 소유하며 중복 Add는 기존 index의 entry를 교체한다. root의 version을 별도로 검사하는 분기는 없다.

AudioSystem은 SoundBank, VoiceAllocator, AudioRng, BgmFader, 설정값을 갖고 Scoped Backend를 소유한다. Backend는 ma_engine, 미리 확보한 ma_sound voice 배열, init 여부 배열, 별도 BGM ma_sound를 갖는다. stb_vorbis와 MINIAUDIO_IMPLEMENTATION은 AudioSystem.cpp 한 translation unit에서 구현한다. Windows 파일경로는 wide-character API를 쓴다.

Init는 먼저 Shutdown 후 bank를 이동하고 음량을 [0,1]로 제한한다. 기본 동시 voice 16, 같은 sound 최대 4, master/sfx 1, BGM 0.6이다. voice와 dryRemaining 배열을 초기화한다. enabled=false 또는 device 열기 실패면 초기화된 무음 모드로 계속한다. device 성공 후 SFX 파일은 resource manager에 DECODE로 선등록하고 BGM은 이 선등록에서 제외한다. effect 재생 때 ma_sound 초기화는 여전히 수행하며 라이브러리 내부 할당이 없다고 주장할 수 없다.

근거: [SoundBank](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:9>), [Backend 소유권](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:17>), [AudioConfig](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:54>), [Init](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:115>).

### 6.1 voice 선택 순서

VoiceAllocator의 slot은 active/soundId/start order를 갖는다. Acquire는 먼저 동일 sound의 active count가 maxPerSound 이상인지 본다. 그러면 그 sound의 가장 오래된 slot을 재사용한다. 그렇지 않으면 가장 앞의 빈 slot을 고른다. 빈 slot이 없으면 모든 sound 중 가장 오래된 slot을 고른다. 선택한 기존 active 여부를 bStolen으로 반환하고 새 order를 기록한다. Release는 slot을 초기값으로 돌린다. Reset은 capacity/maxPerSound를 최소 1로 제한한다.

이는 우선순위 값이나 거리 기반 voice 선택이 아니다. 현재 API의 capacity/count/index는 int이고 무효 slot은 -1이다. Docs/RULES의 새 코딩 표준에 적힌 size_t/kInvalidIndex와 현재 구현이 일치한다고 단정하지 않는다. 이 학습 작업에서는 수정하지 않았다.

근거: [VoiceAllocator.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:4>), [Acquire](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:19>).

### 6.2 SFX와 BGM 처리

Play는 기록 모드이면 요청 event를 먼저 넣는다. 초기화 확인→name 찾기→음량=request×sfx×master→0이면 반환→voice acquire→stolen voice uninit→dryRemaining 0.5초 설정 순서다. 무음 모드는 여기서 끝나고 Update의 dt로 dry slot을 해제한다. device가 있으면 NO_SPATIALIZATION과 DECODE(또는 entry.bgm이면 STREAM)로 ma_sound를 만들고 volume/pitch를 설정해 start한다. 알 수 없는 effect name은 이름별 한 번 trace한다.

pitch RNG는 게임 RNG와 다른 xorshift32이며 shift는 13/17/5다. 상위 24bit로 [0,1)값을 만들고 variation을 [0,0.9]로 제한해 `1+variation×(2u−1)`을 사용한다. device가 있고 variation>0인 실제 재생 경로에서만 이 RNG를 소비한다.

BGM은 voice pool 밖의 ma_sound 하나를 STREAM으로 읽고 loop와 gain을 적용한다. 상태는 Stopped/Playing/FadingOut이다. Playing 상태에서 같은 name을 PlayBgm하면 반환한다. ChangeBgm은 현재 것을 fade-out한 뒤 pending name을 시작한다. 두 BGM을 겹쳐 crossfade하는 구조는 아니다. 이미 fade 중이면 pending name을 바꿀 수 있다. Fade 완료 Update는 pending을 복사→StopBgm→pending을 PlayBgm 순서다. volume은 master×bgm×fade gain이다.

Update는 SFX at_end(무음이면 dry time)부터 정리하고 BGM fade/natural end/volume을 처리한다. Shutdown과 backend destructor는 초기화된 voice/BGM을 uninit한 뒤 engine을 uninit한다. App은 실제 frame delta로 AudioSystem::Update를 호출하므로 BGM fade를 StageRunner의 정수 tick 예약과 같은 시간계로 설명하지 않는다.

근거: [Play](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:255>), [BGM 전환](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:400>), [Update](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:197>), [RNG/Fader](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:3>), [frame dt](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:426>).

### 6.3 요청 순서도 동작이다

StageState에서는 아이템을 collect하면서 pickup 소리를 요청한다. combat resolve 뒤 placeholder kill sound를 요청하고, enemy sounds→enemy events→prop events→player stats/FX의 소리 요청 순서가 이어진다. RequestSounds_ 안에서는 증가한 shots→knife→bomb throws→deaths→폭발 FX→weapon pickup 순이다. knife는 누적 횟수의 홀짝으로 두 소리를 번갈아 쓴다. 마지막에 prevStats/prevWeapon을 저장한다.

같은 tick에 요청이 몰릴 때 이 순서는 voice의 start order와 어느 소리가 교체되는지에도 영향을 준다. bRecord/events는 요청을 검사하는 테스트용 관찰 기능이다. backend의 실제 waveform이나 장치 재생 성공을 기록한 값이 아니다. 이번 작업에서 테스트는 실행하지 않았다.

근거: [Stage event 소비 순서](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:459>), [enemy/prop sound](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:654>), [player sound 순서](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:762>).

## 7. 담당 전체 파일의 역할 표

표의 h/cpp는 같은 책임의 선언과 구현이다. Editor 28개, Audio 8개, Tuning 2개, StageData/Params/Runner/Mapping 8개로 기본 담당 파일 46개를 확인했다. 참조 경계를 위해 AssetPath/JsonFields/EnemyDefLoader/EnemyDef/PropDef/Animation/SpriteLibrary/App/SandboxState/StageState/StageWorldHost와 Assets도 추가로 읽었다.

| 파일 | 핵심 타입·함수 | 책임 |
|---|---|---|
| [Editor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:20>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:13>) | Init/Frame/DrawEditorWindow_/OwnsKeyboard | ImGui 생명주기, 탭, 입력·렌더 조율 |
| [JsonScan.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:5>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:52>) | JsonNode/JsonScanner/FormatJsonNumber | 원문 token span과 숫자 표기 |
| [AnimDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:19>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:126>) | Load/Serialize/SetPivot/Undo | 클립 데이터, 재직렬화, clip snapshot |
| [AnimEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:14>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:17>) | Poll/SyncLibrary_/DrawCanvas_ | Animation UI·preview·mtime, 문서 일부 접근/apply 구현도 포함 |
| [StageDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:73>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:288>) | Reparse_/SetPosition/Duplicate/MoveSection/Patch_ | Stage 원문·색인·텍스트 undo |
| [StageEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:62>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:214>) | LoadStage_/DrawView_/DrawPanel_ | 배치·section·rect 편집과 texture preview |
| [GroundEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:17>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:99>) | GroundMaskDocument, GroundEditor | PNG pixel 모델·stroke undo/redo·그림 도구 UI |
| [EnemyLab.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:17>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:72>) | Spawn/RespawnLast/ReloadDefsKeep/TakeSteps | 샌드박스 재현·선택·정지·감시·overlay |
| [HitboxDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:26>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:127>) | SetValue/ApplyExtraBoxes/Revert | 판정 원문 patch·saved text 복원 |
| [HitboxEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:13>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:117>) | ApplyLive_/DrawEntries_/DrawExtraOverlay_ | 판정 입력·정확한 live 대상·월드 drag |
| [MotionDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:25>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:342>) | ApplyWeapon/SetPartDepth/ListParams/SetParam | 무기·클립·부품·params 원문 patch |
| [MotionEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:12>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:137>) | Save_/DrawParams_/DrawWeapon_/DrawOverlay | 행동·발사 preview, params 즉시 저장 |
| [DepthEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:15>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:90>) | Rebuild_/Patch_/PatchPart_/DrawParts_ | 렌더 depth 목록과 부품 override |
| [ImGuiSdlRendererViewports.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.h:7>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:57>) | RenderTarget/MoveToWindowOrigin | detached ImGui 창 렌더링 |
| [Tuning.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:274>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:66>) | Tuning/Visit_/TuningWatcher | 기본값·공통 field 방문·sandbox 리로드 |
| [StageData.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:295>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:773>) | StageData/StageParser/BoundsCurve | 스키마·검증·소유되는 장면 값 |
| [StageParams.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:3>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:53>) | ParamValue | JSON 문서 수명과 독립적인 중첩 값 |
| [StageRunner.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:21>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:210>) | Tick/Execute_/ProcessPending_/ProcessGates_ | trigger·예약·gate·camera·warp 순서 |
| [StageMapping.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.h:11>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:46>) | MakePropSpawnParams/MakeColliderBox/LoadGroundMask | 데이터 좌표·override·PNG를 실제 월드 값으로 변환 |
| [SoundBank.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:1>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:9>) | SoundEntry/SoundBank | 이름·index·파일경로 |
| [VoiceAllocator.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:4>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:19>) | Acquire/Release/FindOldest_ | voice 수 제한·재사용 순서 |
| [AudioMath.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:5>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:3>) | AudioRng/BgmFader/ComputePitch | 별도 난수·fade gain·피치 범위 |
| [AudioSystem.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:65>) / [cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:115>) | Init/Play/ChangeBgm/Update/Backend | bank·pool·장치·SFX/BGM 생명주기 |

## 8. 이해를 확인할 질문

1. 왜 StageRunner의 pending 포인터가 가리키는 StageData를 실행 중 마음대로 교체하면 안 되는가?
2. 공간 repeat와 시간 wave repeat를 동시에 쓰면 몇 번, 어느 위치에서 생성되는가?
3. gate의 spawn 기록이 없는 경우와 빈 기록인 경우는 왜 다른가?
4. Map 저장, Animation revision 반영, Motion params 저장은 각각 언제 실행 중 값에 영향을 주는가?
5. 숫자 하나의 길이가 바뀌어도 다른 JSON token 위치를 보존하려면 왜 뒤에서부터 patch하는가?
6. voice pool이 꽉 차지 않았는데도 동일 sound voice를 교체하는 경우는 언제인가?
7. BGM fade가 두 음원을 겹치는 crossfade가 아닌 이유는 어떤 필드와 순서에서 확인되는가?
8. 화면상 저장 문구, 요청 event 기록, parser 성공, 실제 재생 결과는 각각 무엇을 증명하는가?

읽기만으로 확인한 구조·수치·호출 순서는 위 링크가 근거다. 저장 성공, 장치 재생, 모든 UI 조작, 녹화 버전과 현재 코드의 동일성은 이번 작업에서 실행으로 확인하지 않았다.
