# Vehicles 구현 결과 (J1~J5, B1~B3)

증거 이미지: `Vehicles_stage.png` (위 3줄 = 지프, 아래 2줄 = BTank, 오프스크린 소프트웨어 렌더, Debug 빌드, `--replay walk135.rep --warp-x 5340 / 4990`).

## 지프
- J1 `VehicleJeep.cpp` `ParkSide_`: 활성화 순간 지프 x 가 "왼쪽 주차 지점 (뷰 min + edgeInset) 이하" 또는 "오른쪽 주차 지점 이상" 이면 IdleAndMove 없이 바로 Move (가까운 가장자리, facing = 그쪽으로 달리는 방향). 뷰 안 스폰은 기존 1 s 대기 경로 그대로.
  - 계획의 "뷰 밖" 판정만으로는 부족했다: river_mid 스폰 시점의 카메라는 락 글라이드 중이라 뷰 min 이 5237 이고, 스폰 x 5297 은 뷰 안 60 px 이다 (판정 실패 -> 오른쪽 끝에서 쏨, 캡처로 확인). 그래서 "주차 지점 안쪽이 아니면" 으로 넓혔다.
  - 첫 주행 (`aux2 == 0`) 의 IdleAndMove 결정에서도 같은 판정을 쓴다 (카메라가 더 뒤처진 경우 1 s 후에도 가까운 가장자리로 간다). 한 번 주차한 뒤 (`aux2 = 1`) 는 기존 규칙 (플레이어 반대쪽 가장자리로 떠남).
- J2/J3 `firstWaitSec` (기본 0 = 기존 동작) = 1.5, `edgeInset` 75 -> 55. speed 200 유지.
- J4 `Stage0.json` river_jeep x 5335 -> 5297 (그 숫자만, cmp 로 확인: 세 바이트 `533`->`529` `5`->`7` 만 다름).
- J5 지프 왼쪽 정지점 (뷰 min 5357 + 55 = 5412) 이 btank (x 5400) 와 겹친다. 캡처에서 지프가 전차 뒤에 완전히 가려진다 (btank 앞에 그려짐). 아무것도 옮기지 않았다. 사용자 결정 필요.
- 캡처 (btank 없이 `--warp-x 5340`): 지프가 왼쪽 밖에서 달려 들어와 (t60) 왼쪽 가장자리에서 서고, 한 번 돌아 (t90~110) 발사대가 오른쪽을 향한 채 로켓을 오른쪽으로 쏜다 (t500~540, `enemy.shot vx +298.5`). 리플레이 플레이어가 중간에 죽어 첫 `enemy.shot` 로그는 늦다 (사망 중엔 FireWeapon 이 쏘지 않음). 단위 테스트의 정지 -> 첫 로켓 = 5.46 s.

## BTank
- 2026-10-12 사용자 결정으로 좌우 방향 전환 제거 (원작은 좌향 고정). Turn 상태/params/`artFacing`/muzzleFx `mirror` 를 btank 에서 뺐다. 엔진의 `artFacing`/`ShouldFlipArt`/FX `mirror` (기본 꺼짐) 는 남겨 두었다.
- TURNING 6 프레임 (sprites.json 추가 필요) 은 쓰지 않았다. 나중 작업.
- 알려진 부작용: 미러 축이 발 원점이라 궤도 중심이 20 px 옮겨 보인다 (Turn 연출에 묻힘).

## 검증
- Debug 빌드 exit 0, 경고 0.
- SlugTest 905 통과 / 3 실패 = 기존 `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor`, `StageIntegration.Boss*` 2 개.
- 새 테스트: VehicleJeep 2 (왼쪽/오른쪽 화면 밖 진입), VehicleBTank 5 (우향 + 그림 반전, 좌향은 반전 없음, 연발 중 안 돎, 히스테리시스, FX 반전), StageDataReal 1 (river_jeep x 5297).
- Regress: `enemylab_fight` 7b12d9c08892fd47 불변. `stage0_walk` c9be5effe93e52e2 -> 01ef3a7debcb5d77: 데이터 (Vehicles.json, Stage0 x) 를 되돌리고 J1 판정을 끈 빌드는 c9be5effe93e52e2 PASS (플래그 전부 끄면 옛 동작) 였으므로 변화는 이 작업뿐. `-Update` 로 갱신.
