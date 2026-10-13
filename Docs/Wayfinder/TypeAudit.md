# 타입/유틸 JugX 대응 점검 (결과 불변 기준)

| MS2 심볼 | JugX 대응 | 차이 | 결정 |
|---|---|---|---|
| kPi/kDeg2Rad/kRad2Deg, 리터럴 3.14159265f | kPI/kDeg2Rad/kRad2Deg | 동일 값 | 교체 |
| Min/Max/Clamp/Lerp(float)/Abs | 동명 | 동일 식(Abs 는 호출부 정수/비교만) | 교체 |
| std::min/max | Min/Max | 동률/NaN 처리 다름 -> 인자 순서 맞바꿔 동일 | 교체(인자 swap) |
| std::clamp/abs(float,int)/sqrt(float) | Clamp/Abs/Sqrt | 동일(fabs=부호비트, sqrtss) | 교체 |
| std::floor/ceil (int 캐스트 직전) | Floor/Ceil | 캐스트 후 동일(오버플로 포함) | 교체; float 결과로 쓰는 곳은 유지 |
| std::atan2(float) | ATan2 | 런타임 atan2f 호출 동일 | 교체 |
| Vec2 | VECTOR2 | 기본 생성자 미초기화(=0 명시), operator/ 스칼라 호출 0건 | 교체 |
| HashBytes + 로컬 FNV 루프 4곳 + RGBA 해시 | Fnv1a | 동일 | 교체 |
| ScopedPhaseTimer/App 시계 | Timer | 로그/dt 만 영향 | 교체 |
| ifstream/ofstream 전수 | FileReader/FileWriter | 오류 코드 동일, 쓰기 경로는 ANSI 변환 | 교체(공용 헬퍼) |
| InputFrame.held 비트마스크 | IndexedFlags<eAction>(uint16) | 동일 비트, 리플레이 포맷 동일 | 교체 |
| 열거형 인덱스 ARRAY 4곳 | DIRECT_ENUM_ARRAY | Count 슬롯 +1 | 교체 |
| 테스트 std::set/map | RbSet/RbMap | 같은 타입 | 교체 |
| UiSystem snprintf | jug::Format | 동일 출력 | 교체 |
| RGBA | jug::RGBA | uint64 union, 상위 바이트 미초기화, == 64비트 | 유지 |
| RandRange | Random<float> | double vs float 나눗셈 | 유지 |
| std::round/lround/fmod/sin/cos/tan/hypot | Round/Mod/Sin.. | 근사/다른 반올림 | 유지 |
| Aabb | AABB | 3D center/extents | 유지 |
| FixedStepClock | 없음 | - | 유지 |
| InputHistory | RingBuffer | 동적할당, 슬롯 1 예약, 빈 상태 의미 다름 | 유지 |
| 적/소품 슬롯 | HandleStorage/ResourcePool | swap-erase 로 순회 순서 변화 | 유지 |
| PropWorld/ItemWorld/StageWorldHost/DemoState 해시 | Fnv1a | 바이트 단위가 아닌 워드 단위 알고리즘 | 유지 |
| PlayerRig kSlots/SlotTable | DIRECT_ENUM_ARRAY | Count 슬롯 때문에 static_assert 깨짐 | 유지 |
| StageData EnumEntry 표 | EnumRefl | 이름 규칙 다름(camelCase) | 유지 |
| atoi/atof/strtod | ParseAs | 공백/+ 허용, 실패 로그 | 유지 |
| snprintf 대체 ZFormat | ZVFormat | 첫 글자 덮어쓰는 결함 | 유지(Format 사용) |

## 미완료: 다음 컨텍스트에서 이어서
- 마지막 커밋 이후 커밋 단위 Verify 확인 (uidemo 캡처는 벽시계 의존으로 간헐 실패, 재시도 필요).
- Release Verify/빌드 exit 미확인.
- std::floor 를 float 결과로 쓰는 곳(Renderer/Camera/에디터)은 -0 문제로 유지, 필요 시 개별 검토.
