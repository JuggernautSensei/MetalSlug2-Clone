# Assets 포맷 (손으로 편집하는 단일 소스)

런타임은 `Assets/` 만 읽는다. 파일을 고치면 곧바로 반영된다.
구 파이프라인과 원본 리소스는 솔루션 루트 `Trash/` 에 있다 (되살리지 않는다).

## 레이아웃
```
Assets/
  manifest.json   아틀라스 페이지 목록 + 다른 JSON 파일 이름
  anim.json       애니메이션 클립 (프레임 사각형 + 피벗)
  sprites.json    정적 스프라이트 / 큰 배경
  sounds.json     효과음/BGM 이름 -> wav
  Atlas/          atlas_000.png ...   (anim.json/sprites.json 의 "atlas" 번호 = 파일 번호)
  Textures/       독립 PNG (배경, 큰 스프라이트, 픽셀맵)
  Audio/          wav
  Ground/         스테이지 충돌 마스크 PNG (Stage0_0.png ...)
  Defs/ Stage/ Tuning/   게임 데이터 (Docs/EnemyDefsSchema.md, StageFormat.md 참고)
```

## anim.json: 오프셋 직접 수정
```json
"Player_Stand_Idle_Lower_Right": { "loop": true, "loopStart": 0, "frameDuration": 0.06,
  "cellW": 85, "cellH": 58,
  "frames": [ { "atlas": 0, "x": 10, "y": 20, "w": 40, "h": 52, "pivotX": 21, "pivotY": 50 } ] }
```
- `atlas,x,y,w,h` : 아틀라스 페이지 안의 프레임 사각형 (w=h=0 이면 빈 프레임).
- `pivotX,pivotY` : 엔티티 원점이 프레임 좌상단 기준 어디인지. 그릴 때 `topLeft = origin - pivot`.
  - 스프라이트를 오른쪽으로 옮기려면 `pivotX` 를 줄이고, 아래로 옮기려면 `pivotY` 를 줄인다.
- `frameDuration` 초 단위 (전 프레임 동일), `loop`, `loopStart` (루프 시 되돌아갈 프레임).
- 클립 이름은 코드/Defs 가 참조한다. 이름을 바꾸면 `Scripts/ValidateStage.ps1` 로 확인한다.
- 좌/우는 별도 클립 (`_Left`, `_Right`). 미러는 없다.

## sprites.json
```json
"Stage0_Main": { "file": "Textures/Stage0_Main.png", "w": 14094, "h": 1328, "columns": 1, "rows": 1 }
"PressStartUI": { "atlas": 2, "x": 0, "y": 0, "w": 160, "h": 24, "pivotX": 80, "pivotY": 12 }
```
`file` 이면 독립 PNG (`columns/rows` 로 격자 분할), `atlas` 이면 아틀라스 사각형.

## sounds.json
```json
{ "version": 1, "sounds": { "Stage1_BGM": { "file": "Audio/Stage1_BGM.wav", "bgm": true } } }
```
새 소리 = wav 를 `Audio/` 에 넣고 한 줄 추가. `bgm` 생략 시 효과음.

## Ground/*.png (충돌 마스크)
- 크기 = 스테이지 월드 픽셀 (예: 14094x1328). 1픽셀 = 1 월드 픽셀.
- **파랑 (0,0,255) 불투명 = 땅**, 흰색/투명 = 공기. 판정은 `R<80, G<80, B>=200, A>=128`.
- 아무 이미지 편집기로 칠하면 된다 (안티앨리어싱 끄기). 파일 이름은 `Defs`/`Stage` JSON 의 지면 이름과 같다.
- 로더: `LoadGroundMask` (`Source/Game/Stage/StageMapping.cpp`).

## 확인
- `Scripts/Regress.ps1` : 빌드 + 리플레이 해시 비교 (`Docs/Wayfinder/Baseline.txt`).
- `Scripts/ValidateStage.ps1` : 스테이지가 참조하는 스프라이트/클립/소리 이름이 존재하는지.
