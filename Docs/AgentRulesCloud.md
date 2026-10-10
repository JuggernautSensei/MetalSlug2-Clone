# 클라우드(Linux) 에이전트 규칙

`Docs/AgentRules.md` 의 프로젝트/코드 규칙은 그대로 따르되, 빌드/실행은 Windows 가 아니라 아래 Linux 절차를 쓴다.
코딩/설계 스타일은 `Docs/CodingStyle.md` 가 최우선이다 (C 배열 금지, `auto` 금지, 람다는 `<algorithm>` 인자로만).
전체 작업 계획은 `Docs/RemakePlan.md`.

## 빌드 / 테스트 / 실행

- 의존성: `/opt/deps` (SDL3, yyjson, stb_vorbis). 세션 훅 `.claude/hooks/session-start.sh` 가 설치한다.
- 구성: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/deps`
- 빌드: `cmake --build build` (CMake 는 소스를 glob 하므로 파일 추가/삭제 후 재구성만 하면 된다)
- 에셋 굽기: `./build/bin/SlugBaker --legacy Resources` (`Assets/Baked` 가 없거나 시트/정의가 바뀌었을 때)
- 테스트: `xvfb-run -a ./build/bin/SlugTest` (필터: `--gtest_filter='Suite.*'`). 기존 테스트는 모두 통과 상태를 유지한다.
- 게임 캡처: `xvfb-run -a -s "-screen 0 1280x720x24" ./build/bin/MyMetalSlug2 --no-audio --state stage0 --capture <prefix> --capture-ticks a,b,c`
  -> `<prefix>_<tick 5자리>.bmp`. PNG 로 변환해 Read 도구로 본다. 상태 이름: `title`, `stage0`, `sandbox`, `demo`.
  그 밖의 옵션(`--replay`, `--record`, `--warp`, `--warp-boss`, `--weapon`, `--life`, `--seed`) 은 `Source/Core/App.cpp` 참고.
- Windows 프로젝트 파일: 소스 파일을 추가/삭제/이동하면 `python3 Scripts/GenerateProjects.py` 로 `.sln/.vcxproj/.filters` 를 갱신한다.

## 원작 영상

- `Assets/Source/videoplayback (1).mp4` (496x360, 30fps), `videoplayback (2).mp4` (480x360, 29.5fps). Mission 4 전체 플레이.
- 도구와 장면 타임라인: `Scripts/Video/`, `Docs/VideoTimeline.md`.

## 작업 규칙

- 남의 레인(지시받지 않은 파일)은 수정하지 않는다. 공개 API 를 바꿔야 하면 바꾸지 말고 보고서에 적는다.
- 커밋은 지시받은 경우에만 한다.
- 보고: 만든/수정한 파일, 테스트 결과, 시각 검증에서 본 것, 알려진 문제.
