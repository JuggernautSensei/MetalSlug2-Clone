# 최종보스 전함 '빅 셰이' (Big Shiee)

코드: `Source/Game/Enemies/BigShiee.cpp` (+`BigShieeCfg.h`, `BigShiee.h`), 정의: `Assets/Defs/BigShiee.json`, 시트: `Assets/Source/40204.png` (`Scripts/GenBossSheet.py` -> `Scripts/ReplaceBossSheet.py`), 연습장: `bosslab` / `bosslab_god` / `bosslab_bot` (`Assets/Stage/BossLab.json`, `Scripts/GenBossLab.py`).

## 구조 (시트 실측)
- 선체 셀 (271x208, 시트 1x, 게임 2x) 은 **포탑이 없는 본체**다. 정답 이미지(`Docs/img/BigShieeReference.png`) 에서 포탑 스프라이트(Idle 첫 프레임)를 템플릿 매칭해 앵커를 실측했다 (픽셀 일치 msd 0). 선체+포탑 합성과 정답 이미지 차이는 궤도 위상/선미 포신/안테나 뿐이다.
- 포탑 7 문: 쌍발 A,D(오른쪽 선수 쪽, 오른쪽을 본다) / B,C(왼쪽 위) , 단발 E,F,G (갑판 아래). 앵커(선체 셀 바닥 가운데 기준 게임 px): A(84,-254) B(-87,-234) C(-113,-184) D(134,-220) E(-18,-144) F(-82,-112) G(-136,-124). 안테나 패널 2 조각 P(-43,-336) Q(65,-318).
- 선수(X 문양)는 오른쪽. 선체 원점 = 셀 바닥 가운데 (피벗 `origins`), 모든 선체 열이 같은 원점.

## 원작 영상 실측 (V1 4:47~6:14)
- 입장: 선수부터 왼쪽 밖에서 약 13 s 에 걸쳐 들어와 선수가 화면 폭의 약 88~92% 에 선다 (`entranceKeys`, 화면 기준 위치 표). 이후 월드에 고정되어 카메라와 함께 왼쪽으로 흐르고, 선미가 200px 아래로 가면 선수를 385px 로 되돌리는 돌진(약 9 s 주기).
- 카메라: 지면 스크롤 약 16 영상px/s = 20 게임px/s, 멈춤 구간 있음 (4:52, 5:03.5, 5:07, 주포 전개 중 9.5 s, 5:25, 5:44). `scrollPauses`/`deployPause*`/`latePause*`.
- 포탑 탄: **청록/빨강/노랑 점 하나가 2프레임씩 깜박이는 곡사탄** (비행 약 3.3 s, 올라갔다 포물선, 한 번에 한 문 쌍발 0.17 s 간격, 포격 간격 1.0~2.8 s). 착탄 시 `PlayerBombExplosion` 불기둥 (원작의 불기둥). 청록 길쭉한 물방울은 플레이어 슬러그 발칸이다 (프레임 대조로 확인). 시트의 Boss_Dot 3색 5프레임이 탄 스프라이트.
- 포탑 일제: 준비(0.3 s) -> 오른쪽 포탑은 왼쪽으로 회전(Turn 9프레임) -> 발사(Shot, 연기 폭발 Boss_Blast) -> 복귀.
- 포탑이 다 부서지면 선수가 열리고(D1->D3->D6->D9->Open) 주포가 올라온다 (Deploy 14 -> Aim 9 -> Shot 5행, 플레이어 각도로 행 선택). 주포 3 s 간격 회색 고폭탄(Boss_Missile 3 각도, 연기 꼬리, 착탄 폭발+판정).
- 파괴: 주포 상단 화염 -> 검은 원 -> 선체 연쇄 폭발(약 6.3 s) -> 잔해(Boss_Wreck+CannonWreck) + 연기 구름 -> 8.4 s 뒤 destroyFinished.

## 수치 근거/가정
- 체력은 원작에서 알 수 없어 시간으로 맞췄다: 포탑 단계 약 20 s, 주포 약 55 s (권총/HMG 평균 17~20 dps 기준: 포탑 40~50, 주포 1200).
- 착탄 충격파로 플레이어를 띄우는 효과는 구현하지 않았다 (PlayerController 에 넉백 API 없음, 폭발 판정 피해만).
- 카메라 락 사각형은 보스가 매 틱 요청하는 스크롤만큼 옮겨진다 (`Camera::ShiftLock`, `StageRunner::AddCameraScroll`). 플레이어는 락 사각형 안(화면 안)에 갇힌다.

## 스테이지 데이터 (Stage0.json boss_arena)
락 `[13700,864,14340,1312]` glide 1, spawn `bigshiee` (x 는 무시, y 는 지면, facing right), gate `bigshiee_dead` (destroyFinished) -> unlockCamera, clear, playerInvincible 10, endStage.
