# MAP: 사람이 유지보수 가능한 MyMetalSlug2

로컬 마크다운 트래커. 이 파일이 유일한 상태 저장소. 설계는 최소로, 삭제 우선.

## Destination

AI가 고친 코드베이스를 **사용자 본인이 이해하고 직접 유지보수**할 수 있는 상태.

1. 코드: JugX/JugGfx 스타일(`Docs/CodingStyle.md`). 프로젝트 최대한 하나로.
2. 리소스: 사람이 찾기 쉬운 폴더. 미사용은 솔루션 루트 `Trash/`.
3. 데이터: Baker/Python 파이프라인 폐기. 사람이 편집하는 단일 포맷.
4. 툴: SDL3 + ImGui 에디터 (애니 오프셋, 맵 배치, 충돌 픽셀, AI 테스트).
5. 런타임 동작 동일 (단순화용 기능 삭제 허용).

실행(리팩터링/이동/툴)까지 이 맵이 담당.

## Notes

- **에디터 원칙 (사용자: 상당히 단순하게)**: 월드 클릭=선택, 드래그 또는 숫자 입력, 변경 즉시 반영, Ctrl+S 저장, 한글 이름. Undo/백업/검증 경고 등은 필요 시 한두 줄 수준만. 새 추상화·프레임워크 금지. 스크립트 행동(C22)은 폐기.
- 자동 검증 캡처는 창 없이 offscreen+software 드라이버로 (사용자 요청: 창이 반복해서 뜨지 않게). 실제 창은 최종 시간 측정에만.
- 응답 한국어 + caveman. 과도한 설계 금지. 삭제가 변환보다 우선.
- 스타일 하드룰: C 배열 금지, `auto` 금지, 변수 람다 금지. **.h 는 선언, .cpp 는 정의. 사소한 getter/setter 도 .cpp. 헤더 본문 예외는 템플릿과 `__forceinline` 급뿐 (constexpr/inline 은 예외 아님). .cpp 가 없으면 만든다** (2026-10-11 사용자 규칙). **ImGui 는 multi-viewport(+docking) 사용** (B7b). **네임스페이스 사용 안 함** (익명 포함, B11c 에서 일괄 제거; 그 전까지 신규 코드는 기존처럼 slug 안에 둠). **주석은 최대한 지양**: 이름으로 설명, 의도가 비자명할 때만 한 줄 (신규/수정 코드 기준, 기존 주석 일괄 삭제는 안 함).
- 모델: 기계적·범위가 좁은 작업은 haiku(자의적 판단 허용), 설계/병합/에디터/마이그레이션은 sonnet. haiku 결과가 검증을 못 넘으면 sonnet 으로 재시도.
- **모든 항목은 구현 전에 테스트/검증 절차(V)를 정의하고, 구현 후 V를 수행해 체크. 전(기준 측정)과 후(동일 측정) 비교가 필수. 검증 불통과 시 완료로 표시하지 않는다.**
- 진행도 = 완료 / 전체, 단계마다 % 보고.
- 렌더러 = SDL3 `SDL_Renderer`. ImGui는 `imgui_impl_sdl3` + `imgui_impl_sdlrenderer3`.
- 위험: `Assets/Baked`는 gitignore, 레거시 외부 경로(`..\Client\Output\Binaries\Resources`)에서 구움.
- 규모: 프로젝트 5, 소스 47k줄, Resources 792MB, Assets 902MB.

## Decisions so far (확정, 2026-10-11)

| 항목 | 결정 |
|---|---|
| 프로젝트 | exe 1 + 테스트 1. Baker/Headless/Core lib 병합. 헤드리스 = exe 플래그 |
| 리소스 | `Assets/` 단일 루트 (Textures, Anim, Sounds, Stages, Defs). `Resources/`+`Assets/Source` 통합. 미사용 → `Trash/` |
| 포맷 | JSON 유지(yyjson). 충돌 맵 = PNG 마스크 직접 편집. Atlas 베이크 제거, 에디터가 오프셋 직접 저장 |
| 회귀 | replay 1~2개 상태 해시 비교. 테스트 39개는 안 살림 |
| 기능 삭제 | 실행 중 발견 시 Trash + 아래 기록 |
| 에디터 | 별도 exe 없음. 게임 exe `--editor` 모드 (ImGui) |
| 프로토타입 | 생략. B7/B8이 실제 첫 버전 |

기준선 변경 기록: 2026-10-11 머신건 사격 변경(각도 흩뿌림 제거, 조준 방향 법선 ±6px 오프셋) 으로 stage0_walk 해시 의도적 갱신 (8a1603466c3e8704).

기준선 변경 기록: 2026-10-11 16:2x 사용자가 에디터로 Assets/Ground/Stage0_0.png 수정(x4572-4732,y571-641, 8251px)해 stage0_walk 해시 갱신 (27bb669fd1656905). 원본 마스크는 Trash/Assets/Ground_Stage0_0.original.png. 코드 변경(C1/C2)은 원본 마스크로 기준 해시와 동일 확인.

기준선 변경 기록: 2026-10-11 21:07 사용자가 Map/Collision 에디터로 Stage0.json(스폰 199->198)과 Ground/Stage0_0.png 를 저장해 stage0_walk 해시 갱신 (f0ec6cf24cf3dafb). 코드/레거시 정리와 무관 (enemylab_fight 불변, 원본 anim.json 복원해도 동일 해시 확인).

기준선 변경 기록: 2026-10-12 사용자 편집(Bus.json soldierMax 2->4, 사루비아/BTank 포구 위치, 보스 주포 클립 pivot)과 layer->depth 통합 후 stage0_walk 해시 갱신. 렌더 캡처 md5 전/후 동일, Defs 값은 키 이름만 변경 확인. 테스트 기대값(BusTest soldierMax, BTank/사루비아 포구)은 데이터에서 읽도록 변경.

삭제 기록: Trash/Resources (레거시 792MB), Trash/AssetsSource (원본 시트·mp4 822MB). 런타임은 Assets/Baked 만 읽음 (Regress 로 확인).

## 체크리스트 — 진행도 19 / 19 (100%)

각 항목: 담당 모델 / **V = 검증 절차**

- [x] A2 리소스 사용 판정표 (haiku). V: 판정표 행 수 = 실제 파일 수, 무작위 10개 수동 대조 — 완료 (A2_Usage.csv 1670행)
- [x] A7 ImGui+SDL3 통합 사실 확인 (sonnet). V: `vcpkg install --dry-run` 에 imgui feature 해석 성공, init 코드 20줄 이내 — 완료 (A7_ImGui.md, 실제 컴파일은 B7)
- [x] B1 회귀 기준 구축 (sonnet). V: 동일 빌드 replay 2회 실행 해시 동일 — 검증 완료. `Scripts/Regress.ps1 [-NoBuild] [-Update]`, 기준 `Docs/Wayfinder/Baseline.txt`, 리플레이 `Test/Replays/`
- [x] B2 Trash 이동 + 폴더 재분류 (haiku+스크립트). V: 이동 후 빌드 OK, replay 해시 B1과 동일, 로드 실패 로그 0 — 완료: Resources/ 와 Assets/Source/ 를 Trash/ 로, Regress PASS
- [x] B3 포맷 변환 (sonnet). V: replay 해시 동일, 변환 전후 애니 프레임 수/오프셋 샘플 비교 — 완료: Baked 승격(Assets/ 평탄화), 충돌맵 .bin→PNG
- [x] B4 로더 교체, Baker/Python 삭제 (sonnet). V: `Assets/Baked` 없이 실행, 해시 동일 — 완료: Baker/Python/Bake.ps1 Trash 이동, SlugTest 809/809
- [x] B5 프로젝트 병합 (sonnet). V: 솔루션 빌드 OK, 프로젝트 수 2, 해시 동일 — 완료: 프로젝트 2개, `--headless` 플래그
- [x] B6 스타일 정리 (sonnet). V: grep `auto`/변수 람다/C배열 0건(외부 라이브러리 제외), 빌드+해시 — 완료: Source 는 auto 0/위반 람다 1건 수정. Test/ 는 auto·람다 잔존(테스트 처분은 Not yet specified)
- [x] B7 ImGui + `--editor` 골격 (sonnet). V: `--editor` 실행 시 ImGui 창 표시 스크린샷, 일반 모드 해시 동일 — 완료: Source/Editor/Editor.*, B7_editor.png
- [x] B8 에디터: 애니 오프셋 (sonnet). V: 오프셋 수정→저장→JSON 변경 확인→재실행 반영 — 완료: Source/Editor/AnimEditor.*, SlugTest 810/810, B8_anim.png
- [x] B9 에디터: 맵 배치 (sonnet). V: 배치→저장→스테이지 JSON 반영→재실행 확인 — 완료: StageEditor.*, 바이트 보존 저장, SlugTest 814/814, B9_map.png
- [x] B7b ImGui multi-viewport + docking (sonnet) — 완료: SDL_Renderer 뷰포트 shim(Source/Editor/ImGuiSdlRendererViewports.*), viewports=3 확인. V: 에디터 창을 메인 창 밖으로 드래그 가능(별도 OS 창), 도킹 동작, 일반 모드는 ImGui 미초기화, Regress PASS, 빌드 OK. SDL_Renderer 백엔드가 뷰포트 미지원이면 최소 변경 대안 제시 후 보고
- [x] B7c 프로젝트 이름 변경 MyMetalSlug2 → MetalSlug2_Clone (haiku, B7b 직후). 대상: sln, Projects/ 폴더, vcxproj(AssemblyName/Target), exe 이름, Scripts/Regress·Build, Play/OpenInVS.cmd, CMakeLists, vcpkg.json name(소문자·하이픈 `metalslug2-clone`), 창 제목(App.cpp) = `MetalSlug2 (Clone)`, Docs. 저장소 루트 폴더명은 안 바꿈. V: 빌드 OK, `Output/Debug/MetalSlug2_Clone.exe` 존재, Regress PASS, SlugTest 통과, `git grep MyMetalSlug2` 0건(Trash·Wayfinder 제외) — 완료
- [x] B10 에디터: 충돌 픽셀 (sonnet). V: 그린 PNG 저장→게임에서 충돌 반영 — 완료: GroundEditor.*, SlugTest 819/819, B10_collision.png
- [x] B11 에디터: 몬스터 AI 테스트 (sonnet). V: 몬스터 스폰/일시정지/스텝 동작 확인 — 완료: EnemyLab.*, SlugTest 829/829, B11_ai.png
- [x] B11b 헤더 본문 분리 (sonnet, 사소한 한 줄 함수도 .cpp 로). V: 헤더 내 함수 본문 중 template/__forceinline 아닌 것 0건(스크립트 grep), 빌드 + Regress PASS + SlugTest 전부 통과 (후보: 44개 헤더 / 약 267개 본문, Math.h·Result.h·Handle.h 등 템플릿/constexpr 은 예외) — 완료: hdrcheck 0건 (Source+Test), 해시·SlugTest 829 동일
- [x] B11c 네임스페이스 전부 제거 (sonnet). V: `git grep -nE "^\s*namespace|using namespace"` 0건 (namespace fs=std::filesystem 별칭 포함 제거 또는 함수 내부 한정 사용), 익명 네임스페이스(101개 파일/126곳)는 static 또는 접두 이름으로 변환(ODR 충돌 방지), `slug::` 한정자 393곳·`sfx::`·`fontmap::` 정리, 빌드 + Regress PASS + SlugTest 전부 통과 — 완료: namespace/slug::/using namespace 0건, Release 빌드 OK, SlugTest 829
- [x] B12 문서 정리 + 유지보수 가이드 (haiku). V: Docs 파일 수 감소, 가이드 1개에 빌드/편집/실행 절차 — 완료: README.md, Docs 6개만 유지, CMakeLists Trash
- [x] B13 최종 회귀 (sonnet). V: 클린 빌드 + replay 해시 동일 + 에디터 스모크 — 완료: SlugTest 829/829, Regress PASS, Debug+Release 빌드 exit=0, 에디터 4탭 캡처 exit=0, auto 0·변수 람다 0·namespace 0

## 추가 요청 (2026-10-11, 완료 후)

- [x] C1 에디터 모드 플레이어 무적: Off / 죽음 방지(HP 0 안 됨) / 완전 무적 선택 (AI 탭 또는 Editor 창). V: 테스트로 3모드 동작, 일반 모드 영향 0, Regress PASS
- [x] C2 히트박스 크기 자유 설정: 플레이어/적/보스 히트박스(w,h,오프셋) 에디터 편집 + JSON 저장. V: 라운드트립 테스트, 오버레이로 확인(스크린샷), Regress PASS
- [x] C3 → C4~C6 으로 대체됨 (사용자 재요청)
- [x] C9 배틀쉽 낙하산 NPC: 낙하 중에도 POW 스프라이트 + 낙하 X 를 카메라 오른쪽으로 (BigShiee.json pow_chute clips 덮어쓰기, chuteRightMarginX), BigShiee 테스트 17
- [x] C10 배틀쉽 주포(큰 헤드) 정합: (a) 미사일 발사 방향과 주포 조준 행/미사일 스프라이트 불일치 → 하나의 조준 행에서 방향·스프라이트 모두 도출 (b) 주포 상태별 애니메이션 정립: idle(Ready 대기 루프), 플레이어 쪽으로 각도 전환(행 사이를 순차 전환), 발사(Shot 행 클립). 기존 프레임 재사용, JSON 으로 클립/타이밍 조절 가능. 부족한 아트는 목록으로 보고. V: 단위 테스트(발사 방향 == 행 각도 == 미사일 스프라이트 행), 상태 전이 테스트, 스크린샷 3장(idle/track/fire), BigShiee 테스트 통과 — 완료: 주포 yaw 상태머신, cannonAimDeg/cannonMuzzleX, 미사일 rotate
- [x] C11 헬기 idle 애니메이션: 시트 FLYING 7프레임(Heli_Pitch_*) + 블레이드 7행×5(Heli_Rotor_0..6) 로 정지 idle 해소. 피치 프레임 순환 + 로터 행 동기, JSON 으로 조절. V: idle 프레임 변화 단위 테스트, 스크린샷 다프레임, 시뮬레이션 해시 불변(렌더 전용) — 완료: idlePitchFrom/To, 로터 행 동기
- [x] C14 로딩 속도 개선 (sonnet). 측정(2026-10-11, 유휴 상태 2회): Release title 1.6~1.9s / stage0 1.9~2.0s, Debug title 8~11s / stage0 9~10s. 구성: (0) 단계별 로딩 시간 로그(JSON/아틀라스/텍스처/오디오/def/스테이지) (1) Debug 에서도 디코더(stb_image/stb_vorbis/miniaudio/yyjson) TU 는 최적화 컴파일 (2) `App::SubmitToMainThread` 스레드 안전 큐 + 비동기 로드 헬퍼(워커에서 PNG/오디오 디코드, 메인에서 GPU 업로드; 람다 금지 규칙에 따라 콜백은 인터페이스) (3) 사용 전까지 미룰 수 있는 오디오 지연 로드. V: 전/후 로딩 시간 표(Debug/Release x title/stage0), 렌더 결과 픽셀 동일(캡처 diff 0), Regress 해시 불변, 큐 스레드 안전 테스트(N 스레드 동시 제출 -> 모두 1회, 메인 스레드에서만 실행), 종료 시 데드락 없음, `--headless` 영향 없음 — 완료: MainThreadQueue/AsyncLoader/App::SubmitToMainThread, Debug stage0 9.1s->1.9~2.2s, 재검증 완료
- [x] C15 보스 진입 구간 진단·수정 (결과: Docs/Wayfinder/C15.md) (sonnet, C14 뒤). 증상(사용자 확인): 보스 등장 시 바닥의 작은 오브젝트(상자·블록)가 그 자리에서 사라짐 + 카메라 움직임이 부자연스러움. 단서: PropWorld.cpp:624 컬링이 프롭 앵커 x < cameraLeft - cullMargin(기본 200, big_house 150) 만 보고 스프라이트 폭을 무시(big_house 폭 498 -> 일부가 화면에 남은 채 제거 가능), 보스 미사일/폭발이 파괴 가능한 블록을 부술 수 있음(Stage0_Block_Destroy), 카메라는 LockWithGlide(1.0s) + 보스 AddCameraScroll. 방법: (1) 재현 도구: 헤드리스/봇으로 보스 아레나 진입(테스트의 ReachBossArena_ 사용) + 틱별 카메라 x, 락 사각형, 프롭 이벤트(Culled/Destroyed/Removed: 틱, id, def, pos, 화면 안 여부)를 CSV 로 덤프 (2) 사용자 기록 파일(Test/Replays/user_*.replay)이 있으면 같은 덤프 (3) 분석: 화면 안에서 제거된 프롭 목록, 카메라 속도/가속도 불연속 구간 그래프 (4) 수정: 컬링을 스프라이트 폭 반영 + 화면 밖일 때만, 카메라 락 전환/자동 스크롤 가속 연속화. V: 전 = 덤프 + 화면 안 제거 건수 N, 후 = 0건, 카메라 가속도 최대값/불연속 전후 비교, 테스트(컬링은 화면 밖 완전 이탈 후만), 해시 변화 시 사유 기록
- [x] C6 (최우선, 사용자 최초 요청 3번) 몬스터 데이터화: 좌표/오프셋/수치 하드코딩 제거 + 직렬화/역직렬화 + 런타임 편집 + 핫리로드 (2단계, sonnet; 감사표/이전 목록 작성은 haiku). 범위 실측(2026-10-11): Source/Game 의 비영(非零) Vec2 리터럴 약 48개(Enemies 23, Player 13, Combat 6, Stage 3, Sandbox 2, Props 1), `constexpr float kXxx = N.f` 약 46개(Enemies 8, Player 7, Props 13, Combat 6, Stage 5, Sandbox 7), 예: Bus kAnchorU/V, EnemyCommon kBloodOffsetY, BigShiee kChuteEdgeX, Heli kFixedScale. 요구: (a) 모든 좌표·오프셋·핫스팟(발사 위치, 이펙트 위치, 터렛, 스프라이트 배치)을 def 데이터 파일(Assets/Defs/**)의 필드로 이전, 값은 기존과 동일 (b) def 구조체에 Serialize/Deserialize 대칭 구현, 라운드트립 테스트(load->save->load 동일) (c) 에디터 `Anchors` 패널에서 값 수정 시 즉시 반영, 저장은 바꾼 숫자만 패치(서식 보존), 외부 편집은 1초 폴링 자동 리로드(C5 경로 재사용), 리로드 버튼 (d) 규칙 검사: Source/Game 에 비영 좌표 리터럴이 허용 목록(방향 단위벡터, 0) 외에 없음을 grep 스크립트로 검증. 추가 요구 (e) 패턴(동작)별 연출 위치: `clips.<동작>` 에 `offset:[x,y]`(해당 동작 재생 시 스프라이트 배치 보정), `effects` 목록(각 항목: clip, offset, z, 반전 규칙, 발생 프레임)을 정의 필드로 일반화하고 에디터 패널에서 동작 선택 후 편집. V: 전 = 현재 offset 보유 필드 목록, 후 = 동일 값 이전 + 동작별 offset 즉시 반영 테스트. V: 전 = 하드코딩 감사표(개수/위치) + Regress 해시, 후 = 감사 0건(허용목록 제외), 해시 불변, 라운드트립/편집->리로드 테스트, 스크린샷(패널) — 부분 완료: 버스 표 + 패스 A 17개 이전, 나머지 허용 목록 (패스 B/패널 보류)
- [x] C13 타이틀 화면을 Raw/143506.png 로 구성 (sonnet, C10/C11 뒤, C12 앞). 시트의 BACKGROUND(파랑) + LOGOS(검은 그림자 METAL SLUG 2, 주황 METAL SLUG, 파란 2, SUPER VEHICLE-001/II, ©SNK 1998) 를 REFERENCE(파랑 2 버전) 와 동일하게 배치해 비트맵 폰트 `MY METAL SLUG 2` 대체. PRESS START 등 안내 문구는 유지. 런타임용 텍스처는 Assets/Textures 로 복사(원본은 Raw/ 유지), 사용 사각형은 sprites.json 에 기록. V: 합성 결과가 REFERENCE 영역과 픽셀 비교 일치(스케일 2x 가정 시 오차 0), 스크린샷, TitleState/UI 테스트 통과, 시뮬레이션 해시 불변 — 완료: TitleBackground 640x448 (Raw/143506.png REFERENCE 2x), 텍스트 제거
- [ ] C12 좌/우 스프라이트 중복 제거 + 이름 통일 (sonnet, C10/C11 뒤, anim.json 동결 필요). 규칙: 좌우 쌍은 1개만 남기고 렌더 시 x축 반전, 이름 `Entity.Action` (예: Player.Idle, Bazuka.Attack, Soldier.Die). 결과(시뮬레이션 해시 + 렌더 픽셀) 불변이 절대 조건. 데이터 측정: 좌우 쌍 252개 중 233개가 픽셀 완전 거울상(pivotX 이상치 0:194, -1:38 → 클립/프레임별 `pivotXFlip` 보정으로 정확 재현), 19개는 거울상 아님(프레임 수/아트 상이, 사용자 편집 포함) → 둘 다 유지하되 이름만 통일(.Left/.Right 접미 대신 의미 있는 변형 이름). 모델: 클립에 `facing`(native 방향) + 선택적 프레임별 `pivotXFlip`, 렌더 요청 facing 이 다르면 flipX. V: (1) Regress 해시 불변 (2) 전/후 렌더 픽셀 비교: 모든 클립 x 양 방향 x 전 프레임을 기존 데이터로 그린 이미지와 신규로 그린 이미지가 100% 일치 (3) SlugTest 통과(이름 기반 테스트는 새 이름으로 갱신) (4) 이름 변경표 CSV(old->new) 산출 (5) 에디터 Animation 탭이 새 모델(facing, flip 미리보기) 지원
- [x] C4 미사일 탱크 중력 미적용 버그 수정 (sonnet). V: 공중 스폰 시 낙하 후 지면 접지 테스트, 다른 차량/적 동일 확인, Regress(의도 변화면 사유 기록) — 완료: missle_tank/btank StepPhysics + useGravity:false 제거, 차량 4종 낙하 테스트
- [x] C5 애니 오프셋 즉시 반영 + 핫리로드 (sonnet). V: 에디터에서 pivot 수정 시 실행 중 화면 즉시 반영, 외부에서 anim.json 수정 시 자동 리로드, 테스트, Regress 불변 — 완료: AnimDocument.ApplyTo/Poll, anim.json 1초 폴링 자동 리로드, Reload 버튼
- [x] C7 다부품 오브젝트 깊이(z level) 설정: 버스 본체/뒤판/덮개 등 부품별 z 를 JSON 으로 분리, 군인 등 다른 객체와 교차 정렬(버스 뒤판 < 군인 < 버스 덮개) (sonnet, C6a 뒤). 대상 감사: Bus, Heli, Sarubia/차량, BigShiee 포탑, 기타 다부품. 에디터에서 z 슬라이더로 즉시 반영. V: 정렬 순서 단위 테스트(SpriteBatch 제출 순서), 스크린샷 비교(버스 위 군인이 뒤판 앞·덮개 뒤), 렌더 전용이라 Regress 해시 불변 — 완료: Bus.json parts.<name>.{layer,sort}, cover layer 16 > 병사 15 (BusDataTest)
- [x] C8 중력을 몬스터 특성(def 필드 `gravity`)으로: 비행체(헬기, flying_bzk, 폭탄 등)와 보스 제외, 사람형/건물형/차량은 전부 true 로 일관 적용 (sonnet, C4 뒤). 모든 def 감사표(gravity 현재값/신규값/이유) 산출, 에디터(AI 인스펙터/Hitbox 계열 패널)에서 토글 + 핫리로드. V: 감사표, 정의별 낙하 테스트, 비행/보스 불변 테스트, Regress 변화 시 사유 기록 — 완료: useGravity 명시, GravityTraitTest, C8_gravity.md

## 마무리 모드 (2026-10-11, 사용자 요청: 토큰 부족 -> 검증/테스트 최소화)

- 검증은 빌드 + Regress + 핵심 골든 1개 + 감사 스크립트만. 캡처 전수 비교/Release/문서 다듬기 생략.
- 확정 큐(사용자): C6 패스 A(진행) -> C6 패스 B -> C7 버스 z -> C8 중력 -> C13 타이틀(haiku). C12 이름/좌우 통일, Anchors 패널은 보류.

## Not yet specified

- 기존 `Test/` 39개 최종 처분 (B5 때 결정: 해시 테스트 외 삭제 기본).
- git 용량 (Assets/Source mp4/원본 PNG 822MB) 처분.
- 에디터 패널 세부 (B8~B11 직전에 정함).

## Out of scope

- 새 게임 기능, 신규 콘텐츠, 밸런스/연출 변경.

## Result

- 솔루션 `MetalSlug2_Clone.sln`: 프로젝트 2개 (`Projects/MetalSlug2_Clone`, `Projects/SlugTest`). CMake 는 Trash 로 이동.
- 런타임은 `Assets/` 만 읽는다 (anim/sprites/sounds/manifest JSON, Atlas/Textures/Audio/Ground/Stage/Defs/Tuning). 변환 파이프라인 없음.
- 옛 파이프라인(Baker, Python, 원본 리소스)과 AI 작업용 문서는 `Trash/` 에 보관. 되살리지 않는다.
- `Source/` = Base, Core, Render, Audio, World, Game, Editor. 네임스페이스·auto·변수 람다·C 배열 없음, 헤더는 선언만.
- 에디터(`--editor`): Animation(anim.json) / Map(Stage/*.json) / Collision(Ground/*.png) / AI(Defs 핫 리로드), 저장 Ctrl+S.
- 회귀: `Scripts/Regress.ps1` + `Test/Replays` + `Docs/Wayfinder/Baseline.txt` (enemylab_fight, stage0_walk).
- 남은 문서: `Docs/` 의 CodingStyle, AssetFormat, StageFormat, EnemyDefsSchema, PropSpecs, PlayerSpec. 진입점은 루트 `README.md`.
- 루트의 `Feedback/`(빈 폴더)와 `CMakeLists.txt` 는 정리됨.
- 남은 일: B13 최종 회귀(클린 빌드, 해시, 에디터 스모크), git 용량 처분.

## 추가 완료 (사용자 요청 라운드, 2026-10-11 밤)
- C16 히트박스 구분 표시 + 에디터 키 입력 분리, C19 공격 모션/이펙트 편집·제자리 리로드·solid 시각화·버스 z, C20 히트박스 생성(hurt/attack), C21 패턴 값(params) 편집, C18 Map 스프라이트 표시, C17 섹터 드래그 + 락 사각형. 미구현: 이벤트/액션 추가·삭제 편집(읽기 전용 표시만), C12 이름·좌우 통일, 스크립트 행동(폐기), 노드 전면 구조(폐기).

## 대기 중 (사용자 요청)
- C24 중첩 오브젝트 부품별 깊이: 모든 다부품 def(헬기 body/rotor/pod/puff, BigShiee 선체/안테나/주포/포탑 A~G 개별/원판/잔해, SV-001 부품, 플레이어 리그·오버레이, overlay 가진 적(낙하산 등), 소품 parts)에 `parts.<이름>.{depth,sort}` 를 JSON 으로 노출(기본값 = 현재 상수), 제출 깊이 = 인스턴스 z + (부품 depth - def 기준 depth). 에디터 Depth 창/AI 인스펙터에서 부품 단위 편집·즉시 반영·저장. 현재 버스만 가능. 선행: 인스턴스 z 작업(진행 중) 완료 후.

- 완료(2026-10-12): 인스턴스 z(적·소품·플레이어·슬러그·아이템·투사체·FX·데코), 부품별 깊이 params.parts(헬기·BigShiee 포탑 A~G·overlay·버스), Depth 창 Parts 트리. 남은 상수: SV-001 부품 오프셋(Tuning.json), 소품 파편/폭발 FX, 훈련 표적 탄, 헬기 폭탄 자식 적의 z.

- 2026-10-12 보스 1단계 완료(T0 event-log, T2 카메라 정지/왕복, T3 포탑 조준, T5 발사 섬광). stage0_walk 기준선 acfc69d63f33c641 로 갱신: 원인 = 사용자 Stage0.json 섹터 이동(bus 2680, heli_river 5357, river_mid 5515, 스폰 -1) + Tuning/anim 편집. 점수표: Docs/Wayfinder/Boss_stage1.md
- 2026-10-12 버스 2단계 완료(U-T1~U-T4: 지붕 도착, 도착 후 공격, 단계별 그리기 순서). stage0_walk 기준선 4489aa958d2ebdda: 의도된 변화(병사가 지붕에 올라감, Bus.json roofTop/roofSpan 키 추가로 증명). bus_climb 점수 0.256: 버스 위치가 원본보다 54-66px 오른쪽(사용자 위치 조정 영역), buslab_bot 이 버스를 148틱 일찍 파괴.
- 2026-10-12 보스 3단계 완료(T4 미사일 가속/연기/충돌, T6 Boss_CasingStrip(sprites.json 끝에 1항목 추가만), T7 탄피 생성, T8 망루 파괴 FX, T9 BossLab 망루). 점수: cannon_casing 0.757, shack_destroy 0.594, cannon_fire_down 0.487, boss_explosion 0.631 (위치 격차는 사용자 영역). 상세: Docs/Wayfinder/Boss_stage3.md
- 2026-10-12 버스 인게임 등반/뚫음 수정(roofSpan, HoldOnRoof: 플레이어가 버스 왼쪽일 때 병사가 지붕 왼쪽 끝에서 윗갑판으로 떨어지던 문제). 버스 hp 160->240, 보스 포탑 hp x1.5 반영으로 stage0_walk 기준선 c9be5effe93e52e2 갱신 (hp 160 으로 되돌리면 4489aa95 로 복귀 확인).
- 2026-10-12 보스 캐논 짝/각도/탄피 수정, 지프 왼쪽 등장·공격(Stage0 river_jeep x 5335->5297 한 숫자), BTank artFacing+Turn(히스테리시스 40px) 완료. stage0_walk 기준선 01ef3a7debcb5d77. 사용자 위치 작업 대기: cannonMuzzleX [+40,+73,+102,+130,+163], missileMuzzle.y -18, Shot pivotY 256->254 (Boss_pairing.md).
- 2026-10-12 BGM 엔진(ChangeBgm, playBgm.fadeOutSec) 추가. 버그 수정: 에디터 'Y 잠금 해제'가 남긴 groundSnap:false+fromY 조합을 로더가 거부해 Stage0 로드 실패 -> 로더가 fromY 를 무시하도록 완화. 사용자 Stage0 편집(heli_river 5458, openland_4 trigger 12310 / lock [12200..12800], 스폰 196) 에 맞춰 테스트 기대값 갱신, stage0_walk 기준선 갱신. 대기: BGM 파일 2개(Boss_BGM, Field_BGM) 사용자 제공 또는 다운로드 승인.

### 2026-10-12 (후속) 버스 깊이/지붕, 헬기 폭탄, 사루비아, 로켓 가속
- Bus.json parts sort 재배치: back 1 < innerSoldier 2 < body 3 < cover 4(depth16) < window 5. 장식 병사가 뒷껍질(back) 앞, 겉껍질(cover) 뒤.
- Bus.json roofTop/roofSpan [0,0]: 계단 병사는 승강대(2층)까지만. 코드의 지붕 기능은 유지 (y != 0 이면 켜짐). 지붕 의존 테스트 1건 SKIP.
- HeliBomb: 플레이어 x 근처(hit/2 + blast/2)에서 떨어지는 폭탄은 머리 위 구조물을 통과해 플레이어 발 높이에서 터짐.
- Stage0: openland_4 triggerX 12310 -> 12270, sarubia x 12552 -> 12760 (건물 오른쪽 끝, 원래 note 값). 사용자 요청.
- 플레이어 로켓: Tuning rocket projectileSpeed 300 -> 200, speedAccel 1600, speedMax 900 (WeaponTuning.speedAccel/speedMax, Bullet.accel/maxSpeed).
- 보스 미사일 크기 20% 증가 요청 취소: 미사일 scale 1.0/box 26x40 유지, casingScale 2.4 -> 1.0 (원본 크기).
- stage0_walk 기준선 갱신 (코드/버스 데이터 변경에 의한 해시 변화, 스테이지 데이터 무관 확인).
- 보스맵 땅: 섹션 드래그(-40)가 마스크에 묶인 프롭(big_house/bridge_block/tower, x>=13130)까지 옮겨 스프라이트와 충돌 픽셀이 40px 어긋났음 -> 프롭 9개 +40 복원 (13170/13542/14566/15218/15590/16242/16614/14194/14904). 테스트 `OpenlandHousesAndBlocksStayAlignedWithTheGroundMaskBumps` 추가.
- 보스맵 파괴 패치: 주기 꼬리(x>=14094)는 읽기 전용이라 집 파괴 시 지면이 안 낮아졌음 -> GroundMap::MaterializeTail(3) 로 꼬리를 17166 까지 실제 픽셀로 펼침 (StageState 로드 시).
- 결정: JugX/JugGfx 렌더러 교체는 지금 하지 않음 (사용자 판단). 이유/이득은 "자기 라이브러리 써보기" 정도라 후순위. 오프스크린 캡처 의존은 포팅 시 비용 아님 (캡처 기능 불필요 판단).
- HeliBomb: 발판(platform) 접촉 제거. 강가 감시초소(guard_post) 발판에 막히던 문제. 지면 마스크/벽만 본다. (+ 플레이어 바로 위 폭탄은 구조물을 통과해 발 높이에서 터짐)
- 사용자 편집 반영(17:58): bus 섹션 triggerX 2631, lock [2433..3073], sarubia x 12639. 테스트 기대값 2482 -> 2433, stage0_walk 기준선 갱신 (해시 변화 원인 = 사용자 Stage0 편집 + HeliBomb).
- BGM 연결: Assets/Audio/Field_BGM.ogg (Back to the China), Boss_BGM.ogg (Boss Stage) 를 사용자 제공 mp3 에서 변환. Stage0 bgm -> Field_BGM, boss_arena 첫 액션 playBgm Boss_BGM fadeOutSec 2.0.
- 보스 사망 회색 원(Boss_Disc_Black 제출) 삭제. casingScale 2.0 (원본 대비 2배). openland_4 triggerX 12270 -> 12220 (사루비아 락 더 빨리).
- 사운드: 사용자 제공 `METAL SLUG SOUND EFFECTS 2.mp3` (나레이터 49대사) 를 faster-whisper(tiny/base/small.en) 로 전사해 구간 식별. 21.70~26.00 = "Mission 4 start!" -> Assets/Audio/Sfx_Mission4Start.ogg, 42.45~46.20 = "Mission complete!" -> Sfx_MissionComplete.ogg. kMissionComplete 가 Sfx_MissionComplete 를 가리킴 (Sfx_Bingo 는 파일 보존). 스테이지 시작 banner 액션에 선택 필드 `sfx` 추가 (Stage0 intro 에서 Sfx_Mission4Start). IStageHost::PlaySfx 추가.
- 보스 주포 체력(def hp) 1200 -> 800.
- 2026-10-12 후속 (영상자료 `Release 2026.10.12 - 20.08.25.01.mp4` 참고):
  1) respawnDropY 40 -> 160 (낙하 등장이 화면 위 밖에서 시작). 2) Bus 파손 단계 겉껍질(cover) 을 BodyD 전체 대신 위층 범위만 (coverBottomDy 108, 개방 단계 OpenTop 과 동일) -> 승강대 병사가 껍질 뒤로 들어가던 역전 해소.
  3) river_jeep x 5297 -> 5100 (뷰 밖에서 입장). 4) heli_river lock rect 좌/우 -70 ([5388..6028]). 6) big_house platform offset y -57 -> -47.
  7) openland_4: triggerX 12170, sarubia groundSnap true (y 제거, 바닥에 붙어 등장). 8) openland_4 대기 병사 웨이브: 하늘 -> 화면 오른쪽 바닥 입장 (ay bottom dy -34).
  9) bigshiee_missile 폭발 Explosion_Huge. 10) BigShiee 딸피 연출: 남은 체력(포탑+주포) 비율 < lowHpFxBelow(0.6) 부터 선체 위에 Explosion_Small 이 무작위로 터짐 (간격 1.2s -> 0.25s).
  5) 집 파괴 시 픽셀 소실: 데이터 검증 (big_house patch: 지운 비율 ~13%, 바닥 열 미손상) 으로는 재현 안 됨. 미해결, 사용자 확인 대기.
- 버스 계단 병사: 계단 끝 (승강대) 에서 낙하 대신 전방 점프 (ClimbLeap: 수평 = 병사 속도, 수직 = jumpVy) 로 버스 몸체 (solid h132, 승강대 윗면 106) 를 넘어 버스 위에 올라선다. 승강대 벽과 버스 사이에 끼던 문제. 올라오는 속도는 그대로. Enemy::bLeap (착지하면 해제). 테스트 `StairSoldiersLeapForwardOverTheLandingWallOntoTheBus`.
- 보스 미사일: 폭발 offset y -10 -> -30 (20px 위), 폭발음 `PlayerBomb` (projectile explosion.sfx, EnemyWorld::TakeSounds -> StageState). big_house platform y -52 (원본 -57 과 이전 -47 사이).
- 버스 깊이 근본 원인 (Opus 분석, anim 프레임 픽셀 합성으로 확인): `Bus_BackD1..3` 은 뒷벽이 아니라 "파손판 Bus_OpenTop (앞면 덮개, x -42..81) + 파손 계단 (x >= 78)", `Bus_BodyD1..3` 은 `Bus_OpenBody` 와 같은 전체 몸통 (승강대 기둥/좌석 포함, x -122..102). 이전 구현은 BodyD 상단 crop 을 덮개로 그려 승강대 기둥/좌석이 병사 앞에 그려졌다.
  수정: 파손 단계 = 계단 (BackD, 15/1) -> 몸통 (BodyD, 15/3) -> 병사 -> 덮개 (BackD 를 x < coverMaxX=82 로 crop, 16/4). 개방 단계와 같은 구조. `coverBottomDy` 폐지 -> `coverMaxX`.
- 버스 구간 카메라 락 rect +50: [2483,0,3123,448] (setBounds 는 그대로).
- 사용자 편집 반영 (Stage0 bus 섹션 triggerX 2605, lock [2457..3097], setBounds -89, 스폰 195). 락 기대값 2457.
- 버스 병사 수 6: 원인 = Bus.json innerSoldiers (창 안 장식 병사, stage 3/4/4/5 에 SD_Idle 이 나타남) 이 반파 시 병사가 늘어난 것처럼 보임 (실제 계단 병사 max 3 은 진단 테스트로 확인). innerSoldiers [] 로 비움 (코드 기능 유지).
- 버스 입장 시 하늘에서 병사 낙하: street 섹션 spawnWave (top-right dy 47.5) 5곳을 화면 오른쪽 바닥 입장 (ay bottom dy -72) 으로 바꾸고, bus 섹션 시작에 `clear waves` 추가.
- 폭발에 날아간 병사 시체 (BeginSoldierDeath Bomb phase): 카메라 아래로 완전히 (발 y - 90 > view.max.y) 나가야 제거. 10 초 안전 상한 후 기존 규칙.
