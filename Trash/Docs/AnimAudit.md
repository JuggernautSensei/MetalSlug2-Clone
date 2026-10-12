# 애니메이션/스프라이트 데이터 전수 점검 (RemakePlan 1-1)

범위: `Assets/Baked/anim.json` 의 모든 클립 (버스 `MainTruck_*`, 헬기 `Aparch_*`, 전함 보스 `Boss_*`/`BattleShip_*`, SV-001 은 재작업 예정이라 제외).
사용자 제보: NPC 가 애니메이션 도중 다른 NPC 로 바뀜 / 적이 전차였다가 갑자기 미사일(발사대) 로 바뀜 / 초반 꼬마와 노인이 깨져 보임.

## 1. 점검 방법

- `Scripts/AnimAudit.py` : 엔티티 계열(클립 이름 첫 토큰)별 컨택트 시트 PNG. 프레임을 피벗(엔티티 원점) 기준선에 맞춰 나열하고 프레임 번호, 루프, 프레임 길이를 표기한다.
  `python3 Scripts/AnimAudit.py --out <dir> [--family SD] [--match Hill]`, 구조 점검만: `--check`.
- 플레이어/병사(SD)/바주카/방패병/지프/BTank/MissleTank/Sarubia/NPC/POW/아이템/이펙트/소품 트럭 계열 시트를 모두 눈으로 확인했다.
- 정의 대조: `Assets/Defs/Enemies.json`, `Vehicles.json`, `Props/Props.json`, `Assets/Stage/Stage0.json` 이 부르는 클립 이름, 프레임 수, 프레임 길이를 anim.json 과 비교.
- 아틀라스 점검: 서로 다른 이미지 사각형이 겹치는지, 다른 계열이 같은 사각형을 공유하는지 (공유는 Older/POW, 건물 창문 파편처럼 같은 그림을 재사용하는 것뿐이었다).
- 실행 점검: stage0 (인트로 구간), vehiclelab, enemylab 을 캡처해 원작 영상(V1 0~15 s) 과 나란히 비교.

## 2. 발견한 문제, 원인, 조치

### 2.1 꼬마/노인(초반 NPC) 가 도중에 다른 사람으로 바뀜 (확정, 수정)
- 증상: 꼬마가 달리다가 멈추는 순간 수염 난 금발 노인(포로)이 되었다가 돌아옴.
- 원인 1: `Stage0.json` 꼬마 시퀀스가 `Order_Run_Left/Right` -> `Order_Turn` -> `Order_Run_Right` 를 이었다. 레거시 `Order_Run_Left/Right` 는 이름과 달리
  **POW 노인(Older) 의 달리기**(90x90, 금발 수염) 이고, `Order_Turn/Idle/Stopping/Run_2` 가 진짜 배달 소년(Delivery Boy, 38098.png) 이다.
  (`pow` 정의의 `run` 슬롯이 `Order_Run_{side}` 를 쓰는 것은 의도된 레거시 이름이라 그대로 둔다. EnemySpecs 4.3.)
- 원인 2: `sheets.json` 에 소년 달리기 클립이 `Order_Run_Right` 로 있었으나 레거시 이름과 충돌해 `SheetNameCollision` 으로 조용히 버려졌고, 이름 토큰도 반대였다 (시트 그림은 왼쪽을 본다).
- 원인 3: 노인(Parker) 시퀀스가 `Parker_Run`(서류가방 든 달리기) -> `Parker_Stopping`(가방을 내려놓는 동작) -> `Parker_Run`(다시 가방) 으로 가방이 되살아났다.
  또 소품 이동 방향과 `flip` 이 모두 반대였다 (원본 아트는 왼쪽을 본다. +x 이동에 flip 없음 = 뒷걸음).
- 조치:
  - `sheets.json`: 소년 달리기를 `Order_Boy_Run_Left` (+ mirror -> `Order_Boy_Run_Right`, 3프레임) 로 새로 자른다. 레거시 이름은 건드리지 않는다.
  - `Stage0.json`: 소년1 = Order_Boy_Run_* + Order_Turn, 소년2 = Order_Run_2 / Order_Idle (오른쪽 이동 구간에 flip), 노인 = `Parker`(대기 9.6 s) 후 `Parker_Run_2` 로 왼쪽 걷기 (원작: 서 있다가 왼쪽으로 걸어 나감).
  - 테스트 `AnimIntegrity.StageDecorSequencesStayWithinOneActor`: 시퀀스의 모든 클립이 존재하고 배우별 허용 목록 안에 있어야 한다.

### 2.2 미사일 전차가 전차로 나타났다가 갑자기 미사일 발사대로 바뀜 (확정, 수정)
- 원인: `Vehicles.json` 의 `missle_tank` 에 `appear` 슬롯이 `MTank_Appear_{side}` 였다. `MTank_*` 는 53213.png 의 **녹색 소형 전차(다른 차량)** 를 잘라 만든 시트 클립이다.
  스폰 후 첫 8프레임(0.67 s) 동안 녹색 전차가 땅에서 솟고, 곧바로 미사일 전차 Idle 로 교체되어 "전차 -> 미사일" 로 보였다.
- 조치: `appear` 슬롯 제거 (원본에서 미사일 전차는 그 연출이 없다). `MTank_*` 클립 자체는 어떤 정의에서도 쓰지 않는다.
- 재발 방지: `AnimIntegrity.EveryDefClipExistsWithMatchingFrameCountAndSingleFamily` 가 한 정의의 클립이 한 계열(첫 토큰) 안에 있는지 확인한다 (인간형 공통 사망 `SD_Die_*` 와 포로는 예외).
- 이미지: `/tmp/scratch/p1anim/final_tank.png` (왼: 수정 전 tick 30 = 녹색 전차, 오른: 수정 후).

### 2.3 레거시 `.anime` 오타/좌우 불일치 (수정: Baker 보정 표)
`Source/Tools/Baker/LegacyFixups.h` 의 표를 굽기 직전에 적용하고 `bake_summary` 에 `LegacyAnimFixup` 로 남긴다.

| 클립 | 문제 | 보정 |
|---|---|---|
| `Player_Stand_Idle_Lookup_SG_Right` | OffsetY -430 (Left -43). 샷건 위보기 대기 자세가 387 px 위로 튐 | -43 |
| `SD_Hang_Idle_Right` | End:0 (1프레임). Left 는 4프레임/0.1 s, 텍스처는 4열 | End 3, 0.1 s (def frames 4/0.1) |
| `SD_Die_Stand_2_Right`, `SD_Die_Land_Right` | 0.08 s (Left 와 def 는 0.1 s) -> 좌우 사망 속도가 다름 | 0.1 s |
| `MissleTank_Down_Idle_Left` | 0.1 s (Right 와 def 는 0.08 s) | 0.08 s |
| `MissleTank_Up_Proj_Effect_Right` | Loop:1 (Left 는 0). 한 번 재생할 총구 이펙트가 영원히 반복 | Loop 0 |

### 2.4 시트 절단 오염 (수정: sheets.json)
- `POW_V1_MoveDown_*`: 다른 줄의 파편 선(75x3)과 큰 푸른 폭발 스프라이트(타 캐릭터) 3개가 프레임으로 잘려 들어옴 -> exclude, 15 -> 12 프레임.
- `POW_V1_Item_*`: 세로 밧줄 줄기 3개(3x48) 가 프레임으로 잘림 -> exclude, 13 -> 10 프레임.
- `MTank_Hill_Driving` 첫 프레임에 시트 라벨 글자 "DRIVING" 이 구워짐. 명시 `frames` 클립에는 `exclude` 가 적용되지 않던 것이 원인
  -> Baker `ClearExcluded()` : exclude 사각형을 시트 키 직후 투명으로 지워 auto/명시 모두에 적용 (SheetSource.h). `53213.png` 에 exclude 추가.
- Enemies.json 의 `moveDown_v1`/`item_v1` frames 값을 새 프레임 수에 맞춤. (POW 변형 V1~V3 는 현재 Stage0 이 쓰지 않는 정의다.)

### 2.5 점검했으나 정상/의도로 판단한 것
- 아틀라스 사각형 겹침 0, 4526 개 고유 이미지. 계열 간 공유 59건은 Older <-> POW(같은 아트), 건물 창문 파편 4방향, Sarubia/BTank 포구 연기, FlameShot/Explosion 첫 프레임 (같은 그림).
- 완전 단색 실루엣 프레임(Explosion 0/1, BTank_Destory 1 등) 은 원작 섬광 프레임이며 Baker 가 부드러운 섬광으로 구움. `Fx_Mine_Ring` 0/1 번째 반원 덩어리도 같은 종류 (미사용).
- `Older_MoveDown` 은 좌우 쌍둥이가 없는 단일 클립 (원본 그대로).
- SD, Bazuka, Shield, Jeep, Sarubia, BTank, MissleTank, 소품 트럭(Truck/LoadedTruck/RefTruck), 아이템 시트는 외래 스프라이트/순서 뒤바뀜/피벗 튐 없음.
- 플레이어 로켓 탄이 `Aparch_Missle_*`(헬기 미사일) 를 그대로 쓰는 것은 `BulletVisual.cpp` 의 설계다 (Combat 레인, 건드리지 않음).
- 이름은 같은데 의미가 다른 레거시 오타 이름(`Order_Run_*`, `BTank_Destory`, `MissleTank_Proj_*`) 은 EnemySpecs 방침대로 유지.

## 3. 자동 점검 (재발 방지)

- `Test/AnimIntegrityTest.cpp` (Assets/Baked 없으면 skip):
  1. 아틀라스 사각형이 겹치지 않고 페이지 안에 있다.
  2. `_Left/_Right` 쌍의 바닥 앵커 차이 <= 64 px, 루프 플래그 동일 (오타 OffsetY -430 같은 피벗 점프).
  3. 정의의 모든 클립이 존재하고 프레임 수가 맞고, 한 정의의 클립은 한 계열이다 (다른 차량 클립 혼입).
  4. Stage0 `anim_decor` 시퀀스의 클립이 모두 존재하고 한 배우의 허용 목록 안이다.
- Baker: `LegacyAnimFixup` 이상 항목, 기존 `SheetNameCollision` 경고(레거시 이름을 시트가 덮지 못하고 버려질 때) 가 리포트에 남는다.
- `python3 Scripts/AnimAudit.py --check` : 아틀라스 겹침 (오류 종료 코드), 클립 내부 크기 급변, 계열 간 공유 목록.

## 4. 남은 것 / 확인 필요

- 원작 영상에서 노인은 0~9 s 에 서 있다가 약 9.8 s 에 팔을 벌리는 자세(Parker 시트 8번째 줄)를 취하고 13.4 s 부터 걷는다.
  현재는 대기(9.6 s) 후 바로 걷는다. 포즈 클립 추가와 정확한 타이밍, 꼬마가 플레이어 곁에 서 있는 배치는 스테이지 레인(2-1) 몫이다.
- 소년1 은 x 1150 에서 달리기만 한다 (원작 위치/동선과 다름 -> 2-1).
- POW 변형(V1~V3)의 Left/Right 피벗이 시트 미러라 일부 프레임이 셀 안에서 한쪽으로 치우친다 (사용처 없음).
- 시트 클립 fps 값은 대부분 임시이다 (SourceSheets.md).
- 정적 소품 스프라이트(Stage0_*) 와 UI 스프라이트는 이번 점검 범위가 아니다 (클립이 아님).
