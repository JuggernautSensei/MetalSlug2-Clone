# Baked 에셋 포맷 (Baker 산출물 ↔ 런타임 계약)

Baker(`SlugBaker`)가 기존 게임 리소스를 이 포맷으로 변환하고, 런타임(`SlugCore`)이 읽는다.
출력 루트: `Assets/Baked/` (git 무시, `Scripts/Bake.ps1` 로 재생성).

레거시 입력 루트:
`C:\Users\Ahnjiwoo\Desktop\Projects\windowAPIProject(24.02.05)\Client\Output\Binaries\Resources\`

## manifest.json
```json
{
  "version": 1,
  "atlasPages": ["Atlas/atlas_000.png", "Atlas/atlas_001.png"],
  "animations": "anim.json",
  "sprites": "sprites.json",
  "sounds": "sounds.json",
  "ground": { "Stage0_0": "Ground/Stage0_0.bin", "Stage0_1": "Ground/Stage0_1.bin" }
}
```
모든 경로는 `Assets/Baked/` 기준 상대 경로, 구분자 `/`.

## anim.json  (프레임 단위 트림 + 피벗)
```json
{
  "version": 1,
  "animations": {
    "Player_Stand_Idle_Lower_Right": {
      "loop": true,
      "loopStart": 0,
      "frameDuration": 0.06,
      "cellW": 85, "cellH": 58,
      "frames": [
        { "atlas": 0, "x": 10, "y": 20, "w": 40, "h": 52, "pivotX": 21, "pivotY": 50 }
      ]
    }
  }
}
```
- 이름은 레거시 `.anime` 의 `File Name` 그대로 (예: `Player_Stand_Idle_Lower_Right`).
- `frames[i]` : 아틀라스 페이지 `atlas` 안의 **트림된** 사각형 `(x,y,w,h)`. 완전 투명 프레임은 `w=h=0`.
- `pivotX/pivotY` : 엔티티 원점(레거시의 `pos`)이 **트림된 프레임 좌상단 기준** 어디에 있는지(px, 정수).
  그릴 때 `topLeft = origin - pivot`. 레거시 규칙으로 계산한다:
  레거시 그리기 `left = pos.x - (cellW/2) + offsetX` (정수 나눗셈), `top = pos.y - (cellH/2) + offsetY`
  → 셀 좌상단 기준 원점 = `(cellW/2 - offsetX, cellH/2 - offsetY)` → 트림 후 `pivot = 그 값 - (trimX, trimY)`.
  (`.anime` 의 `OffsetX/OffsetY`, `Width/Height` 사용)
- `loop`, `loopStart`(레거시 `OptionIndex`: 루프 시 되돌아갈 프레임), `frameDuration`(초, 전 프레임 동일).
- 프레임 인덱스는 레거시 `Start..End` 를 0부터 다시 센 것이다. 레거시 비루프 `Start>0` 컬럼 버그는 **올바른 컬럼**으로 굽는다.
- 좌/우 미러는 굽지 않는다 (원본 `_Left/_Right` 각각 존재).

## sprites.json  (정적 스프라이트 / 큰 배경)
```json
{
  "version": 1,
  "sprites": {
    "Stage0_Main": { "file": "Textures/Stage0_Main.png", "w": 14094, "h": 1328, "columns": 1, "rows": 1 },
    "PressStartUI": { "atlas": 2, "x": 0, "y": 0, "w": 160, "h": 24, "pivotX": 80, "pivotY": 12 }
  }
}
```
- 2048px 초과/배경류는 `file`(독립 PNG, 가로세로 분할 정보 `columns/rows`), 작은 것은 아틀라스 사각형.
- 레거시 `.sprite` 의 `Width/Height/Column/Line` 을 반영.

## Ground/*.bin  (픽셀 충돌맵 → 비트마스크)
리틀엔디안.
```
char  magic[4] = "GRND"
u32   width
u32   height
u8    bits[ceil(width/8) * height]   // 행 우선, 각 행은 바이트 경계 정렬, LSB 가 왼쪽 픽셀, 1 = 고체(레거시 파랑)
```
- 레거시 규칙: 파랑 `(0,0,255)`(알파 255) = 땅. 흰색/투명 = 공기. 안티앨리어싱 가장자리는
  `B >= 192 && R <= 96 && G <= 96 && alpha >= 128` 를 고체로 본다. 임계값은 Baker 옵션이며 리포트에 고체 픽셀 수를 남긴다.

## sounds.json
```json
{ "version": 1, "sounds": { "Stage1_BGM": { "file": "Audio/Stage1_BGM.wav", "bgm": true } } }
```
레거시 `SoundData.txt` 의 이름을 유지하고 원본 wav 를 `Audio/` 로 복사.

## 리포트 (Baker 가 `Assets/Baked/Reports/` 에 생성)
- `contact_<category>.png` : 카테고리별 컨택트 시트(프레임 격자, 피벗 십자 표시).
- `pivot_report.json` : 애니메이션별 `anchorDriftMax`(프레임 간 바닥 중앙 앵커 변동 px), 비정상 항목 목록.
- `bake_summary.txt` : 텍스처/애니/스프라이트/사운드 개수, 건너뛴 항목과 이유.
