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

## 원본 시트 패스 (Assets/Source/sheets.json → anim.json / sprites.json)
스프라이트 리퍼(spriters-resource 류)의 원본 시트 PNG 를 레거시와 같은 아틀라스/anim.json 경로로 굽는다.
`Assets/Source/sheets.json` 이 있으면 `SlugBaker` 가 자동으로 실행한다 (`--sheets <file>` 지정, `--no-sheets` 생략,
`Scripts/Bake.ps1 -Sheets <file>` / `-NoSheets`). `--only` 에 `sheet` 를 넣으면 이 패스만 필터링된다. 카테고리 키는 `sheet`.
구현: `Source/Tools/Baker/SheetSource.h`(알고리즘, inline) / `SheetSource.cpp`(JSON, 패스). 시트 PNG 는 stb_image 로 디코드.

### sheets.json 스키마
```json
{
  "sheets": [
    {
      "file": "46546.png",
      "key": { "mode": "magenta", "tol": 0 },
      "exclude": [[0, 484, 318, 32]],
      "clips": [
        { "name": "Parker_Run", "region": [0, 0, 318, 52],
          "auto": { "rows": true, "mergeGap": 0, "minPixels": 30 },
          "pivot": "foot", "fps": 12, "loop": true },
        { "name": "Order_Run_Right", "frames": [[10, 4, 20, 30], [34, 4, 20, 30]],
          "pivot": "foot", "fps": 10, "mirror": true, "replace": false, "asSprite": false }
      ]
    }
  ]
}
```
- `file`: `sheets.json` 기준 상대 경로(보통 같은 폴더의 PNG). 좌표는 전부 시트 픽셀 좌표(좌상단 원점).
- `key.mode`: `magenta`(±tol 로 (255,0,255)) / `color`(`colors: [[r,g,b],...]` 각각 ±tol) / `alpha`(알파 0 만 배경).
  배경 픽셀은 RGBA 0 으로 바뀌고, 이후 트림은 알파 기준이다. 키 후 알파 > 0 인 픽셀이 전경.
- `exclude`: 시트 전체에서 무시할 사각형(라벨 글자, 크레딧 문구 등). 모든 클립에 적용.
- `clips[].region`: auto 슬라이스 범위. 생략하면 시트 전체.
- `clips[].auto` (`true` 또는 객체): 자르기 알고리즘
  1. region 안에서 알파 > 0 & exclude 밖인 픽셀을 전경으로 한다.
  2. `mergeGap` 픽셀만큼 팽창(가로/세로 윈도 OR)한 뒤 4-연결 BFS 로 덩어리를 나눈다.
  3. 덩어리의 **원본 전경** bbox 를 사각형으로 쓰고, 전경 픽셀이 `minPixels` 미만이면 버린다.
  4. `rows:true` 면 세로 중심이 같은 행(첫 사각형 높이 안)끼리 밴드로 묶고 밴드 안에서 x 순, `false` 면 x 순.
  - 기본값: `rows: true`, `mergeGap: 2`, `minPixels: 30`. 촘촘한 시트는 `mergeGap: 0` 이 맞다(프레임끼리 붙는 경우가 많음).
- `clips[].frames`: 명시 사각형 `[x,y,w,h]` 목록. 있으면 `auto` 보다 우선한다. `auto` 와 `frames` 중 하나는 필수.
- `scale`: 정수 1..8 (기본 1). 시트 항목에 두면 그 시트 클립의 기본 배율이고, 클립에 `scale` 을 두면 그 클립만 덮어쓴다 (클립 값 0 또는 생략 = 시트 값 상속).
  원본 시트 픽셀은 1x 이므로, 레거시 2x 아트와 크기를 맞추려면 2 를 쓴다. **최근접 이웃 확대**를 키 적용 직후, 자르기/트림/피벗 계산 **이전**에 한다.
  따라서 프레임 피벗/트림/cellW/cellH 와 `anim.json` 의 w/h 는 확대된 크기 기준이고, `frames` / `exclude` / `region` 좌표는 여전히 시트(1x) 좌표다.
  자동 검출(`auto`)은 1x 시트에서 수행하고 그 사각형을 확대한다. 미리보기(`--sheet-preview`)는 1x 로 그린다.
- `pivot`: `foot`(기본) = 트림된 프레임의 아래 중앙 `(w/2, h)`, `center` = `(w/2, h/2)`. 피벗 규칙은 위 `anim.json` 과 같다(그리기: `topLeft = origin - pivot`).
- `loop`(기본 true), `fps`(기본 12, `frameDuration = 1/fps`).
- `mirror: true`: 이름 토큰이 `_Right` 또는 `_Left` 인 클립의 반대쪽 클립을 추가로 굽는다
  (`X_Right` → `X_Left`). 프레임은 좌우 반전 이미지이고 피벗 x 는 `w - pivotX`. 토큰이 없으면 경고 후 무시.
  레거시 `_Left/_Right` 는 원본 그대로 각각 구워지며 미러를 굽지 않는다. 시트 클립에만 적용한다.
- `asSprite: true`: 프레임 1개짜리를 `sprites.json` 에 넣는다 (`{atlas,x,y,w,h,pivotX,pivotY}`, 트림된 크기).
- `replace`(기본 false): 같은 이름이 이미 있으면 **이름 충돌 규칙**을 따른다.
  - 이름이 없으면 추가.
  - 이름이 있고 `replace:false` 면 **건너뛰고 경고** (`SheetNameCollision`, 레거시 이름은 절대 조용히 덮지 않는다).
  - 이름이 있고 `replace:true` 면 기존 항목을 무효화(`SheetReplaced`)하고 시트 클립으로 대체.
- 같은 시트 안에서 같은 이름이 두 번 나와도 위 규칙을 따른다.
- 파일은 UTF-8 (BOM 허용). 주석은 JSON 에 넣을 수 없으므로 미작성 시트는 아래 표로 관리한다.

### 미리보기 (검증용 디버그 모드)
`SlugBaker.exe --sheet-preview <file.png> [--sheet-preview-scale 3]` 은 레거시 리소스 없이 실행되며
키 적용 + 자동 검출/명시 사각형에 번호(클립별 0..)를 붙여 `Captures/SheetPreview/<name>.png` 를 쓴다.
`sheets.json` 에 항목이 없으면 마젠타 키 + 시트 전체 auto 로 미리본다. 항목이 있는데 `clips` 가 비었으면 항목의 키로 전체 auto 를 본다.
로그에 클립별 사각형 목록(`index x,y wxh`)이 찍히므로 `frames` 명시 값을 여기서 가져올 수 있다.

### 시트 상태
| 파일 | 내용 | 키 | 상태 |
|---|---|---|---|
| 46546.png | Parker (NPC), 318x516 | magenta | 파일럿: Parker_Run(행 0), Parker_Stopping(행 1), Parker_Run_2(행 9). 나머지 행은 이름 미정 |
| 38098.png | Delivery Boy, 288x472 | alpha | 파일럿: Order_Run_Right(+mirror), Order_Stopping, Order_Idle, Order_Recoil, Order_Turn, Order_Run_2 (시트 라벨 기준) |
| 40204.png | Boss Shiee 보스 본체, 4624x3317 | color (153,217,234) + (0,255,0) | 작성됨: 105 클립 (`Boss_*`), 상세는 `Docs/BossSheet.md` |
| 53213.png | 그린 탱크, 702x3380 | color (153,217,234) | 미작성 (라벨 글자 사이 행) |
| 33825.png | 더블데커 버스 (MainTruck 계열), 567x2199 | color white + 녹색 편집 마커 | 미작성 (마커 exclude 필요) |
| 29720.png | HUD/폰트/GO 표지/MISSION START, 566x1357 | color white/녹색/분홍 배경 | 부분: `UI_Mission_A..Z`, `_1.._9`, `_Excl`, `_Question` (37개, 미션 폰트, `frames` 명시 + `asSprite`). 나머지(GO 표지, 숫자 셋, 게이지)는 미작성 |
| 143506.png | 타이틀/로고 아트, 1506x1860 | color 2색 | 미작성 (스프라이트 아님, 보류 권장) |
| 32522.png | 아이템/이펙트/소형 메카 혼합, 1983x19513 | alpha | 미확인 (식별 후 작성) |

파일럿 주의: `Order_Run_Right/Left` 는 레거시 클립(8프레임, 80x78)과 이름이 겹쳐 `replace:false` 로 건너뛴다.
Running 행을 쓰려면 이름을 바꾸거나 `replace:true` 를 명시해야 한다(레거시 아트 대체). 레거시 `Parker` 는 대체하지 않았다.
fps 값은 임시(원본 재생 속도 미확인)이다.
배율: 46546.png, 38098.png, 40204.png 는 `"scale": 2` (원본 1x 픽셀아트 → 레거시 2x 크기; 예: Big Shiee 본체 271x208 → 542x416).
29720.png 는 1x 유지: `UI_Mission_*` 글자는 `UiLayout.h` 의 `missionGlyphNum/Den`(5/3)으로 코드에서 배율을 보정하므로 이중 확대하면 안 된다.
