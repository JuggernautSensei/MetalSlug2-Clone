# 노드 기반 액터 데이터 설계 (제안)

목표: 몬스터의 모양·위치·깊이·판정·공격·이펙트·패턴을 **하나의 재귀 트리 데이터**로 정의하고, 에디터에서 런타임 편집·핫리로드·저장한다. 코드는 "노드 해석기"만 가진다.

## 1. 기본 원칙
1. 모든 것은 **노드**다. 노드는 자식 노드를 가질 수 있다 (재귀).
2. 좌표·깊이는 **부모 기준 상대값**이다. 부모를 옮기면 자식이 따라간다 (그룹화).
3. 오른쪽 기준으로 저장하고 `facing`이 왼쪽이면 자동 미러한다 (현행 규칙 유지).
4. 재사용: `ref`(다른 노드 트리 포함), `extends`(상속, 현행 유지).
5. 패턴(행동)도 데이터: 상태 노드 + 타임라인 + 전이 조건. 복잡한 보스 패턴은 기존 C++ 행동을 노드에서 이름으로 호출(`behavior`).

## 2. 노드 공통 필드
```
{ "id": "cover", "type": "...", "offset": [x, y], "z": 16,
  "enabled": true, "children": [ ... ] }
```
- `z`: 전역 깊이 값 하나 (현재의 layer+sort를 하나로). 다른 객체(병사 15, 플레이어 20)와 같은 축에서 정렬.

## 3. 노드 타입
| type | 역할 | 주요 필드 |
|---|---|---|
| `group` | 묶음. 이동/활성/z 일괄 | children |
| `sprite` | 그림 | clip, frames, sec, loop, flip(`facing`/none), tint |
| `hitbox` | 판정 | kind(`body`/`hurt`/`attack`/`solid`), w, h, offset, team, hp(옵션: 독립 파괴) |
| `anchor` | 이름 붙은 점 | (offset만) — 발사점, 이펙트점, 터렛 거치점 |
| `emit` | 이펙트/오브젝트 생성 | clip 또는 def, at(`anchor` id), delay/frame, z, once |
| `fire` | 무기 발사 | weapon/projectile def, at(anchor), aim(`given`/`player`/`fixed:deg`) |
| `motion` | 동작(공격 모션) | clip, timeline[events], next |
| `state` | 패턴 상태 | enter[], on[{when, goto}], motion |
| `physics` | 물리 특성 | gravity(bool), speed, maxFall, solid |

`motion.timeline`의 이벤트: `{ "frame": 9, "do": "fire", "weapon": "rocket", "at": "muzzle" }`,
`{ "t": 0.2, "do": "emit", "clip": "Bazuka_Shot_Effect", "at": "muzzleFx" }`, `{ "do": "enable", "id": "attackBox" }`.

## 4. 예시: 낙하산 바주카 병사 (현행 JSON을 노드로 옮긴 모양)
```
{ "id": "flying_bzk", "type": "group", "children": [
  { "type": "physics", "gravity": false },
  { "id": "body", "type": "sprite", "clip": "Bazuka_Fly_Shot_{posture}_{side}", "z": 15 },
  { "id": "para", "type": "sprite", "clip": "Bazuka_Para_Idle", "offset": [-5,-50], "z": 14 },
  { "id": "muzzle", "type": "anchor", "offset": [0,-63] },
  { "id": "hurt", "type": "hitbox", "kind": "hurt", "w": 40, "h": 60, "offset": [0,-30] },
  { "id": "shot", "type": "motion", "clip": "shot", "timeline": [
      { "frame": 1, "do": "fire", "weapon": "bazuka_proj", "at": "muzzle" },
      { "frame": 1, "do": "emit", "clip": "Bazuka_Shot_Effect", "at": "muzzle" } ] }
]}
```
## 5. 예시: 버스 (본체/덮개 분리, 깊이)
```
{ "id": "bus", "type": "group", "children": [
  { "id": "back",  "type": "sprite", "clip": "Bus_BackD",   "z": 15 },
  { "id": "body",  "type": "sprite", "clip": "Bus_BodyD",   "z": 15 },
  { "id": "riders","type": "group", "z": 15, "children": [ { "type":"emit","def":"soldier_sword","at":"window1" } ] },
  { "id": "cover", "type": "sprite", "clip": "Bus_OpenTop", "z": 16 },
  { "id": "window1","type": "anchor", "offset": [-40,-100] },
  { "id": "top",   "type": "hitbox", "kind": "solid", "w": 170, "h": 150, "offset": [0,-75] }
]}
```

## 6. 요구사항 매핑
| 요구 | 노드로 하는 방법 |
|---|---|
| 동작별 스프라이트 위치 | `motion`/`sprite` 노드의 offset |
| 미사일·이펙트가 나가는 위치 | `anchor` + `fire`/`emit.at` |
| 동작 중 오브젝트 생성 | `motion.timeline`의 `emit` |
| 본체/덮개 깊이 | 각 `sprite.z` (그룹 안에서도 전역 z) |
| 히트박스 구분·편집 | `hitbox.kind`별 색, 트리에서 선택 = 월드에서 강조 |
| 중력 특성 | `physics.gravity` |
| 런타임 편집·핫리로드 | 에디터가 노드 값을 직접 수정, 파일 mtime 변화 시 같은 id 노드 값만 교체(인스턴스 유지) |
| 직렬화 대칭 | 노드 <-> JSON 1:1, 저장은 바뀐 값만 패치 |
| 카메라 락/이벤트 | 스테이지 섹션도 같은 `when`/`do` 어휘 (`lockCamera`, `unlock`, `waitDead`) |

## 7. 에디터
- 왼쪽 **노드 트리**, 오른쪽 **인스펙터**(선택 노드 필드), 월드 뷰에 선택 노드를 스프라이트/박스/십자선으로 표시.
- 드래그로 offset, 슬라이더로 z, "Play motion"으로 해당 motion 즉시 발동.
- 노드 추가/삭제/복제/드래그 재배치(부모 변경).

## 8. 런타임 구조 (단순하게)
- 로드 시 트리를 **평탄화**: 노드별 월드 변환(offset 누적, 미러 적용)을 캐시 → 프레임 비용 거의 없음.
- 렌더: sprite 노드를 z 기준 전역 정렬 큐에 제출. 전투: hitbox 노드를 판정 시스템에 등록.
- 시뮬레이션 결정성: 노드 해석은 틱 기반(프레임/시간 -> 틱 환산), 난수는 기존 시드 사용.

## 9. 도입 순서 (기존을 깨지 않는 점진 이전)
1. **어댑터**: 기존 def 필드를 로드 시 노드 트리로 변환 (JSON은 그대로, 동작 불변). 해시 불변이 검증 기준.
2. **에디터 트리/인스펙터**를 어댑터 트리에 붙임 (편집은 기존 JSON 키로 저장).
3. 새 필드(`emit`/`fire` 타임라인, `z`, `anchor`)를 노드 JSON(`nodes` 섹션)으로 추가: 기존 def와 공존.
4. def를 하나씩 `nodes` 형식으로 이전 (버스 -> 헬기 -> 병사류 -> 보스).
5. 마지막에 스테이지 이벤트 어휘 통합.

## 10. 하지 않는 것 (과설계 방지)
- 범용 스크립트 언어/비주얼 그래프: 안 만든다. 조건은 `dist`, `timer`, `hp`, `animEnd`, `dead(id)` 등 고정 목록만.
- 보스 복합 패턴 전체를 데이터로: 기존 C++ `behavior`를 유지하고 그 파라미터와 연출(위치·이펙트·깊이)만 노드화.
