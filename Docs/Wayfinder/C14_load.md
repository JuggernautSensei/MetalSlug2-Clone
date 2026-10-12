# C14 로딩 시간

측정: `MetalSlug2_Clone.exe --no-audio --state <s> --capture-frame 1 --capture x.bmp`, D3D11 실제 창, 벽시계 초 (프로세스 시작 ~ 종료).
단계별 로그는 `[load] <단계> <상세> <ms>` (Base/PhaseTimer.h). 병렬 구간의 단계 합은 벽시계보다 클 수 있다.

## BEFORE (원본, 3회 중앙값)

| 구성 | title | stage0 | 지배 단계 (stage0) |
|---|---|---|---|
| Release | 1.99 s | 1.84 s | png-decode 0.65 s, gpu-upload 0.52 s (플래시 변환본 포함), 창/디바이스 0.3 s |
| Debug | 7.83 s | 9.06 s | gpu-upload 5.0 s (SpriteBatch.cpp 의 플래시 램프 변환이 최적화 없이 돈다), png-decode 2.5 s, ground-mask 0.5 s |

JSON / 애니메이션 / 정의 / 사운드(23개, 약 50 ms) 는 지배적이지 않다.

## STEP 1: Debug 에서도 디코더 TU 최적화

SpriteBatch.cpp (stb_image + 플래시 변환), StageMapping.cpp (stb_image + 지면 마스크), AudioSystem.cpp (miniaudio) 에
Debug 만 `/O2 /Ob2`, RTC/JMC 끔, PCH 미사용 (PCH 와 옵션이 달라 C4653 이 나므로). 두 vcxproj 모두.
Debug title 7.83 -> 1.84 s, stage0 9.06 -> 2.36 s (1회 측정).

## STEP 2: 비동기 로드

- `MainThreadQueue` (Core/MainThreadQueue.*): 뮤텍스 큐. 아무 스레드나 `Submit`, 메인이 `DrainAll`. `App::SubmitToMainThread` 가 여기로 전달. 프레임마다/로딩 대기/종료 시 비운다.
- `AsyncLoader` (Core/AsyncLoader.*): 워커 `min(hw-1, 8)` 개. 작업은 `Execute()`(워커) + `Complete()`(메인, 제출 순서대로). 완료 통지는 MainThreadQueue 로 전달. `WaitAll()`, 제출/완료 카운터, 끝났지만 전달 안 된 결과는 워커 수까지만 (메모리 상한). 워커 0 이면 동기 폴백.
- 적용: 아틀라스 16페이지 (디코드 + 플래시 변환은 워커, 텍스처 생성은 메인) 를 창/렌더러 생성 전에 미리 제출, Stage0 원경/배경 레이어 텍스처 (14094px 포함, 띠 분할은 메인), 효과음 등록.
- `--no-async-load` 로 전부 동기. 핸들/페이지 번호는 동기 경로와 같은 순서.
- 효과음은 miniaudio 리소스 매니저의 잡 스레드 하나가 실제 디코드를 하므로 병렬 이득은 거의 없고 (약 50 ms), STEP 3 (지연 디코드) 은 필요 없어 하지 않았다.

## AFTER (교차 5회 중앙값, 같은 빌드에서 async / --no-async-load)

| 구성 | title async | title sync | stage0 async | stage0 sync |
|---|---|---|---|---|
| Release | 0.76 s | 1.67 s | 1.21 s | 1.94 s |
| Debug | 1.33 s | 1.83 s | 2.15 s | 2.34 s |

BEFORE 대비 (async): Release title 2.6x / stage0 1.5x, Debug title 5.9x / stage0 4.2x.
Release stage0 에 남은 시간: 창/디바이스 생성 ~0.35 s, 지면 마스크(스테이지 PNG 3장 디코드) + Stage0.json 파싱 ~0.4 s 가 메인에서 순차로 돈다.

검증: 렌더 출력 (title/stage0) 은 BEFORE 와 async/sync 모두 바이트 동일 (D3D11 창 캡처 비교, 이후 offscreen+software 에서 async vs sync 동일), Regress PASS (7b12d9c08892fd47 / 27bb669fd1656905).
