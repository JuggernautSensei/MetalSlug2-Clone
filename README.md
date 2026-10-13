# MetalSlug2 (Clone)

C++20 + SDL3 로 만든 메탈슬러그 2 스타일 2D 액션 게임 (Mission 4 한 스테이지).
에셋은 `Assets/` 의 JSON/PNG 를 손으로 고치면 바로 반영된다 (변환 단계 없음).

## 요구 사항
- Visual Studio 2022 (MSVC, C++20)
- vcpkg (manifest 모드, `vcpkg.json`: sdl3, yyjson, stb, miniaudio, gtest, imgui)

## 빌드
- `powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/Build.ps1 [-Config Debug|Release] [-Target SlugTest]`
- 또는 `OpenInVS.cmd` 로 `MetalSlug2_Clone.sln` 을 열어 빌드. 산출물: `Output/<Config>/`

## 실행
- `Play.cmd` : Release 증분 빌드 후 실행. 인자는 게임으로 전달된다.
- `Output/Debug/MetalSlug2_Clone.exe [옵션]`

| 옵션 | 의미 |
|---|---|
| `--state <이름>` | 시작 상태 (`demo`, `sandbox`, `enemylab`, `buslab`, `bosslab`, `helilab` ...) |
| `--seed <n>` | 난수 시드 |
| `--dump-tuning` / `--gen-replay <out> <state> <ticks> <flip>` | 튜닝 덤프 / 합성 리플레이 생성 후 종료 (첫 인자일 때만) |
| `--no-audio` | 소리 끔 |
| `--capture <bmp>` `--capture-frame <n>` `--capture-ticks <a,b,..>` | 스크린샷 |
| `--weapon <n>` `--hold <J/K/Up/Down/Left/Right>` `--life <n>` | 디버그 시작값 |
| `--warp <n>` `--warp-x <x>` `--warp-boss` | 스테이지 위치로 워프 (디버그 중 F5/F6 키도 워프) |
| `--editor` | 에디터 모드 |
| `--editor-tab <animation/map/collision/ai>` | 시작 탭 |
| `--editor-anim <클립>` `--editor-stage <이름>` `--editor-lab-spawn <def>` | 시작 선택값 |
| `--editor-god <off/nodeath/invincible>` | 에디터 Player 무적 모드 초기값 (`--editor` 에서만 적용) |

## 폴더
- `Source/Base` 공용 유틸(Json, Result, Log) / `Core` App, 입력, 리플레이, 튜닝 / `Render` 스프라이트, 카메라 / `Audio` / `World` 지면, 물리, 전투 / `Game` 상태, 플레이어, 적, 소품, 스테이지 / `Editor` ImGui 에디터
- `Assets/` anim.json, sprites.json, sounds.json, manifest.json, `Atlas/` `Textures/` `Audio/` `Ground/` `Stage/` `Defs/` `Tuning/`
- `Test/` GoogleTest (`SlugTest.exe`), `Test/Replays/` 회귀 리플레이
- `Projects/` vcxproj (MetalSlug2_Clone, SlugTest) / `Scripts/` Build, Regress, ValidateStage
- `Docs/` 편집용 문서 (아래 목록) / `Docs/Wayfinder/` 정리 작업 기록
- `Output/` `Intermediate/` `Captures/` 빌드/캡처 산출물 (git 무시)

## 편집 방법 (`--editor`, 저장은 모두 Ctrl+S)
| 탭 | 대상 파일 | 비고 |
|---|---|---|
| Animation | `Assets/anim.json` | 프레임 pivot 오프셋. 포맷: `Docs/AssetFormat.md` |
| Map | `Assets/Stage/*.json` | 스테이지 배치. 포맷: `Docs/StageFormat.md` |
| Collision | `Assets/Ground/*.png` | 충돌 마스크 칠하기 (파랑 = 땅) |
| AI | `Assets/Defs/*.json` | 정의를 손으로 고친 뒤 `Reload defs + respawn` 버튼 (핫 리로드) |

- Animation 탭의 모든 편집(pivot/nudge/Apply to all/loop/frameDuration/Undo)은 실행 중인 게임 스프라이트에 즉시 반영된다 (재시작 불필요).
- `Assets/anim.json` 을 밖에서 고치면 약 1초 안에 자동 리로드 (`reloaded anim.json`). 저장 안 된 편집이 있으면 경고만 하고, `Reload` 버튼이 디스크 내용으로 되돌린다. `sprites.json` 은 대상 아님.
- Editor 창 상단 `Player`: `Off` / `No death` (피격은 받되 죽음/라이프 소모 없음) / `Invincible` (피격 판정 없음). 일반 실행/리플레이는 항상 Off.
- AI 탭의 `Hitbox` 창: 플레이어(`Tuning.json`)와 적/보스(`Defs/*.json`)의 박스 w,h,cx,cy 를 숫자/드래그/`-` `+` 로 고치면 실행 중인 랩에 바로 반영된다.
- `--editor` 에서는 ImGui 창에 포커스가 있거나 키 입력을 ImGui 가 쓰는 동안 게임에는 키가 전달되지 않는다 (게임 화면을 클릭하면 다시 게임 입력). `Hitbox` 에서 편집 중인 박스는 월드에 굵은 외곽선+라벨로 표시된다.
- `Hitbox` 의 `Save` 는 바뀐 숫자만 원본 JSON 에 제자리 교체한다 (서식/키 순서 유지, 편집이 없으면 바이트 동일).

수치 튜닝은 `Assets/Tuning/Tuning.json`. 적 정의는 `Docs/EnemyDefsSchema.md`, 소품은 `Docs/PropSpecs.md`.

- 창 없이 캡처(자동 검증용): 환경변수 `SDL_VIDEO_DRIVER=offscreen`, `SDL_RENDER_DRIVER=software` 를 설정하고 `--capture` 로 실행하면 창이 뜨지 않는다 (렌더 방식이 달라 실제 창 결과와 픽셀이 조금 다를 수 있으니 전/후 비교는 같은 방식끼리만).

## 회귀 확인
- 의도적으로 동작을 바꿨다면 `Scripts/Regress.ps1 -Update` 로 기준 해시를 갱신한다.
- 에셋 이름 확인: `Scripts/ValidateStage.ps1`. 단위 테스트: `SlugTest.exe`.

## 코드 규칙 (자세히: `Docs/CodingStyle.md`, JugX/JugGfx 스타일)
1. `auto` 금지.
2. 변수에 담는 람다 금지 (알고리즘 인자만 허용).
3. C 배열 금지 (`std::array`, `Vector`).
4. 네임스페이스 금지.
5. `.h` 는 선언만. 템플릿/`__forceinline` 만 헤더에 본문을 둔다.
6. 주석은 최소한으로, 필요한 곳에 한 줄.
7. 타입/함수 PascalCase, 멤버 `m_`, 매개변수 `_`, 상수 `k`, `enum class eX`, Allman 중괄호.
8. 에러는 `Result<T>`, 예외 없음.
