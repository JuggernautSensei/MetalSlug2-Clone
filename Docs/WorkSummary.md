# 작업 요약 (클라우드 세션)

브랜치: `claude/affectionate-brahmagupta-tgqzo1`. 계획 전체는 `Docs/RemakePlan.md`.

## 완료

### 실행 환경 (Linux 클라우드)
- `CMakeLists.txt` 추가 (Windows 는 기존 `.sln` 그대로). GCC 호환 수정 2곳.
- `.claude/hooks/session-start.sh`: 세션 시작 시 의존성(SDL3, yyjson, stb, miniaudio, gtest) 설치, 빌드, 에셋 굽기 자동화.
- Baker: 레거시 리소스 경로 대소문자 무시 (`Source/Tools/Baker/PathResolve.h`). Linux 에서 누락되던 텍스처 2개 복구.
- `Scripts/GenerateProjects.py`: PowerShell 없이 `.sln/.vcxproj` 재생성.
- 에이전트 규칙: `Docs/AgentRulesCloud.md`.

### 원작 영상 분석 도구
- `Scripts/Video/` (extract, contact, compare, diffscore): 원작 구간 프레임 추출, 원작 vs 게임 비교 시트, 유사도 수치.
- `Docs/VideoTimeline.md`: 두 영상의 장면별 타임라인과 적 수.

### 코딩 스타일
- `Docs/CodingStyle.md`: JamEngine / JugGfx / JugX 에서 추출한 스타일 가이드.
- 기존 코드 전체 리스타일 (동작 변경 없음: 테스트 전부 통과, 캡처 픽셀 일치, Baker 출력 동일).

### 0단계 정리
- 버스, 헬기, 최종보스의 기존 코드/데이터/테스트/문서 삭제. 보스 아레나 자리는 도착 시 MISSION COMPLETE.

### 1단계
- 사격: 권총/파이어샷/샷건/로켓 연타 전용(홀드 시 1발). 헤비머신건은 1회 입력에 4발 버스트(7틱 간격), 이동/점프/칼/폭탄으로 캔슬.
  파이어샷은 원본 시트 20587 프레임으로 교체. HUD 무기 글자 제거. (`Docs/PlayerSpec.md`)
- 피격 깜빡임: "계속 붉은 상태" 버그 수정. 원작에서 측정한 밝기→주황 램프 색, 3틱 켜짐/3틱 꺼짐. (`Docs/Spec_HitFlash.md`)
- 애니메이션 데이터: NPC 클립 혼선(배달 소년/노인), 미사일탱크 등장 시 다른 탱크로 보이던 문제, 레거시 오프셋 오타,
  시트 잘림 오염 수정. 재발 방지 테스트 `Test/AnimIntegrityTest.cpp`. (`Docs/AnimAudit.md`)

### 2단계 (일부)
- 2-2 최후반 맵 깨짐: 클라우드에서는 재현 안 됨. 유력 원인(가로 14094px 텍스처가 GPU 최대 8192px 초과)에 대응해
  큰 텍스처를 2048px 조각으로 나눠 업로드. 디버그 옵션 `--warp-x X` 추가.

테스트: 776개 전부 통과.

## 미완료 (다음 세션에서 진행)

- 2-1 초반 NPC 연출: 꼬마가 플레이어 옆에 없음, 노인 등장 타이밍/팔 벌리는 동작 누락.
- 2-3 적 배치 전면 재작성: `Docs/VideoTimeline.md` 기준으로 `Assets/Stage/Stage0.json` 재구성 (현재 원작보다 많음, 초반 트럭 위 병사 등).
- 3-1 버스, 3-2 헬기, 3-3 배틀쉽, 3-4 메탈슬러그 SV-001: 작업을 시작했으나 토큰 한도로 중단, 결과물은 반영하지 않음.
  시트: 버스 33825, 헬기 20642, 배틀쉽 40204, SV-001 32522 (`Assets/Source`). 설계 규칙은 RemakePlan 3단계 참고.
- 확인 필요: HMG 버스트 발수(4), 권총 연타 최소 간격(5틱), 파이어샷 속도/사거리(영상에 장면 없음).
- 스타일 가이드 기준 공개 이름 변경 보류분: `IStageHost::PlayerAlive`→`IsPlayerAlive`, `PlayerFeet`→`GetPlayerFeet`,
  `StageWorldHost::GetActiveGroundMut`→`ModifyActiveGround`, `eOrigin`/`ePositionMode` 통합.

## 빌드 / 실행 (Linux)

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/deps
cmake --build build
./build/bin/SlugBaker --legacy Resources
xvfb-run -a ./build/bin/SlugTest
```
