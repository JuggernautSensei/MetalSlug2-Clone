# Depth 축

그리기 순서는 하나의 정수 `depth` 로 정한다 (작을수록 뒤). 옛 `layer` 개념은 없다. 정의 JSON 키는 `depth` (부품 `parts.<name>.{depth,sort}`, 효과 `effects[].depth`, Bus `fxDepth`, Prop `depth`). 옛 `layer` 키는 로드 오류.

| depth | 대상 |
|---|---|
| -1000 | Backdrop (`kBackdropDepth`) |
| -100..-96 | BigShiee 선체/안테나/주포/포탑/사망 원 |
| -90 | Terrain (`world.terrainDepth`, 기본 `kDefaultTerrainDepth`). 이보다 작은 스프라이트는 지형 앞에 먼저 그린다 |
| 4 / 11 | Decor 원경 / 배경 |
| 12 | Prop back (기본) |
| 14 / 15 | Enemy 오버레이(낙하산) / Enemy 몸, 병사 |
| 16 | Bus cover (Bus.json `parts.cover.depth`) |
| 19 / 20 | Slug 차체 / Player |
| 29 | Enemy 투사체 꼬리 |
| 30 / 31 / 32 | Player 탄 / Slug 포탄 / Enemy 투사체, Slug FX |
| 34 / 46 | Item, Slug 화살표 / Score 팝업 |
| 38 / 39 | Prop front / Prop 디브리 |
| 50 / 55 / 60 | Prop 폭발 FX / 탄 소멸 FX / 폭탄 폭발 FX, Enemy FX 기본 |
| 1000 | HUD/UI (`kHudDepth`) |

## 동점 규칙
같은 depth 안에서는 `sortKey` (보통 발 y, 건물은 -x 기반 키) 오름차순, 그다음 제출 순서 `seq`.
`SortSpriteCommands` = (depth, sortKey, seq).

## 에디터
AI 탭의 "Depth" 창이 모든 depth 를 오름차순으로 나열한다. 코드 상수는 읽기 전용, `Defs/*.json` 안의 `depth`/`fxDepth` 값은 DragInt 로 고치고 Save 하면 JSON 숫자만 제자리 교체 + 정의 리로드.

## 인스턴스 z
적(차량/보스 포함)과 Prop 인스턴스는 x, y 처럼 자기 `z` 를 가진다. 기본값은 위 표의 def 기본 depth (적 15, BigShiee 선체 -100, Prop 12/38) 라 JSON 이 그대로면 출력은 같다.
스테이지 spawn 의 선택 키 `"z": N` (정수) 이 기본을 덮어쓴다. 부품 depth 는 `z + (부품 depth - def 기본 depth)` 로 상대 오프셋을 유지한다 (버스 덮개, BigShiee 포탑 등).
맵 탭 Z 필드(기본값과 같으면 키 제거), AI 탭 Spawn/Inspector Z 필드로 편집한다.

플레이어(`PlayerController::SetZ`), SV-001(`SlugVehicle::SetZ`), 아이템(`ItemWorld::Spawn(..., z)`/`SetZ`, 점수 팝업은 같은 이동량), Decor(`DecorSpawn::z`/`SetZ`), 투사체(`BulletSpawn/BombSpawn/ShotSpawn::z`)도 `kDefaultZ` (= 기본 depth 그대로) 를 기본으로 갖는 `z` 를 가진다. 플레이어가 쏜 탄/폭탄은 플레이어 z 의 이동량을 따르고, 파생 FX (탄 소멸/폭발/꼬리) 는 기준 투사체의 이동량 (`zOffset`) 을, 적이 쏜 투사체/FX 는 적 z 의 이동량을 따른다. 효과 (`EffectList::Spawn`) 는 요청마다 depth 를 직접 받는다.

## 부품 depth
정의 JSON `params.parts.<name>.{depth,sort}` (둘 다 선택, 정수, 모르는 이름/키는 로드 오류). 없으면 아래 기본값 (= 기존 상수). 제출 depth = 인스턴스 z + (부품 depth - def 기본 depth).
`sort` 는 같은 depth 안의 정렬 키. 발 y 로 정렬하는 부품 (헬기 body/rotor/pod, overlay) 은 sort 를 발 y 에 더하는 오프셋 (기본 0) 으로 쓴다.

| 대상 | 부품 (기본 depth, sort) |
|---|---|
| Bus | innerSoldier 15/0, wreck 15/0, back 15/1, stairs 15/1, body 15/2, cover 16/3, window 15/4, debris 15/20 (Bus.json 값) |
| Heli | body 15/0, rotor 16/0, pod 16/0, puff 58 (발사 연기 FX) |
| BigShiee | hull -100, antenna -99, cannon -98, turretA..G -97 (sort 0..6), disc -96, wreck -100 |
| `overlay` 클립이 있는 def (낙하산) | overlay 14/0 |
| SV-001 | 몸통+발칸 = z, FX = z + 13, 화살표 = z + 15 (코드 상수, JSON 없음) |

플레이어 리그는 상/하체를 같은 depth 로 그리므로 부품 상수가 따로 없다 (플레이어 z 하나, 시작 낙하산 = z - 1).
Depth 창의 "Parts" 는 def -> 부품 순으로 depth/sort 를 DragInt2 로 고친다 (메모리 def 에 즉시 반영, Save 가 JSON 을 제자리 패치: `parts`/부품이 없으면 기본값과 다를 때만 끼워 넣는다).
