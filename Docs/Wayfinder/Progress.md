> **이 문서는 낡았다. `Docs/HANDOFF.md` 를 먼저 읽을 것.**

# 진행 상황 / 인수인계 (다음 컨텍스트용)

마지막 갱신: 2026-10-13. 브랜치 `claude/affectionate-brahmagupta-tgqzo1`. 태그 `stage1-complete` = 1단계 완료 시점.
작업 주체: 메인 세션이 Sonnet 에이전트들에게 웨이브 단위로 위임(Opus 는 2단계 계획 1회). 이 문서와 `git log` 만으로 이어갈 수 있게 정리함.

## 0. 가장 먼저 할 일 (다음 컨텍스트)
1. `git log --oneline | head -5`, `git status --short | grep -v Trash`, `git stash list`(비어 있어야 함)로 상태 확인.
2. **W12(타입 JugX 교체)는 커밋 완료**(`2a8f843`): 스태시를 복원해 `Verify` PASS(해시 15, 캡처 7, 튜닝, 단위 테스트 902/902)와 `TraceLoad` PASS, Release 빌드 exit=0 확인 후 커밋. 점검표는 `Docs/Wayfinder/TypeAudit.md`.
3. 작업 브랜치 `claude/affectionate-brahmagupta-tgqzo1` 는 `main` 보다 앞서 있고 **아직 `main` 병합/푸시 전**(사용자가 나중에 하겠다고 함): `git checkout main && git merge --ff-only claude/affectionate-brahmagupta-tgqzo1 && git push origin main`.
4. 남은 일: 최종 재평가(4항목), 미참조 에셋을 `Trash/` 로 이동, `Docs/Architecture.md` 최종 갱신(JugX 교체 범위 반영).

## 1. 사용자 요구 (불변 규칙)
- **결과/동작 불변이 절대 조건**: `Scripts/Verify.ps1` → `VERIFY: PASS`(리플레이 해시 15, 캡처 7, 튜닝 해시, 단위 테스트 902/902 통과) + `Scripts/TraceLoad.ps1` → `TRACELOAD: PASS`. 기준선 파일(`Docs/Wayfinder/Baseline*.txt`, `Test/Replays`)은 `-Update` 로 갱신하지 않는다.
- JugX/JugGfx/JugNet 처럼 **졸라 단순하고 평평하게**, C++ 관용구, "웹스러운" 구조(콜백/큐/Job/Notice/인터페이스+가짜/Context 묶음) 금지, 새 클래스/새 파일 금지, 기존 클래스도 얇으면 통합/삭제.
- 헤더 선언 한 줄, 주석은 "왜" 한 줄만(이름 반복·후행·배너 금지). "내가 짰다는 느낌이 풀풀 나야 함".
- 죽은 코드/테스트/리소스/파일은 삭제(`git rm`). **미참조 리소스는 삭제 대신 `Trash/` 로 옮겨 모아둔다(사용자가 나중에 직접 삭제).**
- 싱글턴 허용, `using namespace jug` 는 pch 에서 허용(단 MS2 의 `RGBA`/`Min` 등과 이름 충돌 시 `using jug::X;`).
- 사용자는 **추가 허락 없이 끝까지 자체 진행**을 지시함.
- 상세 기준: `Docs/Wayfinder/StyleRubric.md`(특히 8절 작성자 지문). 메모리: `feedback_*`, `project_ms2_acceptance_criteria.md`.

## 2. 지금까지 한 일 (커밋 순서)
| 구간 | 내용 |
|---|---|
| 백업/검증 | `1a94a68` 백업 스냅샷, `9050654` `Scripts/Verify.ps1` + 합성 리플레이 13개 + 캡처/튜닝 기준선 |
| 1단계 | JugX 서브모듈(`Vendor/JugX`) + `Projects/JugX` 정적 라이브러리, Types/HashMap/Handle, Log 래퍼/Assert/Random, Error/Result, clang-format, static→익명 NS, unique_ptr→Scoped (`stage1-complete` = `1b61f41`) |
| 2단계 S0~S6 | `--trace-load` + `Scripts/TraceLoad.ps1` + `BaselineTextures.txt`, 병렬 디코드 함수(`SpriteBatch::DecodeImages` 등), `AsyncLoader`/`MainThreadQueue`/Job/Notice 삭제 |
| 단순화 W1~W4 | `IAudioSink`/`NullAudio`/`TestAudio` 해체(IStageHost·IGameState 는 유지), `StageLoadDiagnostics` 해체, 사전조건 `JUG_ASSERT`, 내부 `Result` 축소 |
| 삭제 P1~P5 | 죽은 코드/테스트(SlugDef 로더, 플레이어 X 제한, 카메라 흔들림, SKIP 테스트 등), 죽은 문서, 중복 헬퍼 통합 |
| 정리 C1~C3 | 불필요 주석 제거(토큰 동일성 검사로 증명), 헤더 선언 한 줄화, clang-format |
| 리소스 | `Trash/`(tracked 1,586), `Scripts/VideoScore`, `Raw/`, 미참조 이미지, `Sfx_Bingo.mp3` `git rm` (`ebf3656`) |
| W8~W9 | `GameContext` 제거(`g_debug` 전역), **headless 완전 제거**(`--replay <f> --hash` 경로), 서비스 싱글턴(Renderer/SpriteBatch/AnimationLibrary/AudioSystem), 테스트 전역 Environment |
| JugX | 서브모듈 버그 2건 수정 푸시 완료(`c1efc07` → origin/main), 포인터 갱신(`e346435`), 우회 코드 되돌림 |
| 데이터 | `anim.json` 4쌍 `loop=false` 통일(`82f7d42`, 근거 `Docs/Wayfinder/AnimLoop_Analysis.md`) |
| W11 | JSON 을 JugX `JsonDeserializer` 로 교체, 3벌 필드 리더 공용화(에러 문구 통일) (`4d7ba4c`~`0e0904d`) |
| W12(진행중) | 타입 JugX 교체: Math 상수/Min·Max·Clamp·Abs·Lerp·Sqrt, Fnv1a 해시, `IndexedFlags`/`DIRECT_ENUM_ARRAY`, `FileReader/FileWriter`, `Timer`, `Vec2→VECTOR2` (`fd938e0`~`0006ef9`) |

## 3. 현재 수치 (참고)
- 추적 파일 2,036 → 약 430개. Source/Test 는 1단계 이후 대략 −5,500줄 이상 순감.
- 단위 테스트 902개 전부 통과(기존 실패 `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` 는 `anim.json` loop 통일로 해소).
- JugX 대비 스타일 지표 측정 스크립트는 임시 폴더라 휘발됨. 필요하면 아래 지표를 다시 측정: clang-format 일치율, 매개변수 `_` 접두율, 주석 밀도, 헤더 줄바꿈 선언, `JUG_ASSERT`/`Result`/Context 구조체/1000줄 초과 파일 수.

## 4. 남은 일
1. W12 마무리와 직접 재검증(위 0절). 끝나면 `Docs/Architecture.md` 의 "의도적으로 JugX 로 바꾸지 않은 것"(2절 3번)·4절 Base 설명·11절 지표를 최종 상태에 맞게 갱신.
2. **미참조 에셋을 `Trash/` 로 이동** (사용자 규칙): `Docs/Wayfinder/Prune_Assets.md`/`Prune_Assets_Items.csv` 참조. 확정 미사용은 거의 없음(이미 정리됨). 불확실 항목(`Stage/BossArena.json`, 오디오 4개 `PlayerReload`/`PlayerGun_EC`/`Sfx_LetsGo`/`Sfx_HereWeGo`, anim 클립 `Plane`/`Boss_Disc_Black` 등)은 참조 근거를 다시 확인해 정하되, `sounds.json`/`anim.json` 항목은 사용자 편집 데이터이므로 항목 삭제 시 파일과 함께 일관되게 처리하고 Verify 로 확인.
3. 최종 재평가 4항목(JugX 유사성 / 단순성 / 리소스 compact / 동작 불변)을 수치로 보여주고 사용자 요청 산출물 정리: 눈여겨볼 부분, 아키텍처 설명, 프로젝트 설명 .md → `Docs/Architecture.md` 에 있음(갱신만).
4. 남은 구조적 격차(선택): `JUG_ASSERT` 밀도, `Result` 반환 비중, 1000줄 초과 파일 9개(`StageData.cpp` 2.2k, `StageEditor.cpp` 2.3k 등), `IStageHost`+`FakeStageHost`(테스트 시임, 유지 결정).
5. 디스크에 남은 것: `Trash/`(gitignore) 안의 `AssetsSource/*.mp4` 270MB, 중첩 저장소 `Trash/AssetsSource/Metal-Slug-Remake`(522MB), `Trash/Captures`, `Trash/Backup`(여기에 `Captures/tmp/Bus.json.mine` = 사용자 편집본일 수 있는 사본 포함). 사용자가 직접 삭제하기로 함 — 에이전트가 삭제하지 말 것.

## 5. 운영 메모 (함정)
- **권한 분류기**: 대량 파일 삭제(`git rm -r Trash ...`)와 공개 저장소 푸시는 한 번 거부됐고, 사용자가 명시 승인 후 재시도해 성공함. 거부되면 우회하지 말고 사용자에게 명령을 제시.
- **`Assets/anim.json` 은 사용자가 에디터로 편집 중**일 수 있음. 복원/덮어쓰기 금지, 국소 수정만. 마지막 스냅샷은 `82f7d42` 에 커밋됨.
- 기본 빌드/검증: `Scripts/Build.ps1 -Config Debug|Release -Target MetalSlug2_Clone|SlugTest|JugX`, `Scripts/Verify.ps1`, `Scripts/Regress.ps1`, `Scripts/TraceLoad.ps1`. 새 해시 실행: `set SDL_VIDEO_DRIVER=offscreen & set SDL_RENDER_DRIVER=software & MetalSlug2_Clone.exe --replay <f> --hash`.
- `capture_uidemo` 는 벽시계 의존이라 간헐적으로 흔들림 → `Verify.ps1` 이 3회 재시도.
- JugX 헤더는 `<JugX/JugX.h>` 우산 헤더 사용 금지(Windows.h 매크로 `GetObject` 가 MS2 코드를 깨뜨림). 필요한 헤더만 `Base/Pch.h` 에서 개별 include.
- 에이전트 위임 시: 경로를 한정한 `git add`(`Assets/anim.json`, `Docs/Wayfinder/Prune_*`, `Vendor/JugX`, `Trash` 제외), Co-Authored-By 줄, 단계마다 빌드+Verify, 실패 시 해당 단계만 되돌리기.
- 스타일 지표용 스크래치 스크립트와 에이전트 로그는 임시 폴더(휘발). 저장소에는 `Docs/Wayfinder/` 문서만 남음.

## 6. 관련 문서
`Docs/Architecture.md`(프로젝트/아키텍처/자가평가), `Docs/Wayfinder/StyleRubric.md`(정리 기준), `Docs/Wayfinder/Stage2_Plan.md`(Opus 원안 + 개정표), `Docs/Wayfinder/Prune_Code.md`·`Prune_Assets.md`(삭제 조사), `Docs/Wayfinder/AnimLoop_Analysis.md`, `Docs/Wayfinder/TypeAudit.md`(W12).

## 7. 최신 갱신 (리플레이 삭제 이후)
- 리플레이/해시 장치 삭제(`b986871`), 주석 토막화(`0260265`), 헤더 한 줄·const 제거(`86df276`), `.cpp` 값 전달 const 제거. 백업 태그 `backup-before-replay-removal`.
- 사용자 원칙: "돌아가기만 하면 되는" 기준이 아니라 **사람이 읽고 자기 코드 같은지**가 합격 기준. 값 전달 `MemoryView` 는 템플릿 생성자 때문에 const 가 필요(복원함).
- 남음: 큰 파일/`IStageHost`·테스트 시임 정리(구조), `main` 병합·푸시.
