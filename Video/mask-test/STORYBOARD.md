---
format: 1920x1080
duration: 30s
message: "보이는 그림과 밟는 바닥은 다른 데이터라서, 바닥은 빌드 없이 칠해서 고친다"
arc: 챕터 전환 → 개념 → 편집 → 결과 → 결론
audience: 게임 클라이언트 채용 담당자·개발자
mode: collaborative
---

## Frame 1 — 챕터 전환

- scene: 노란 면이 사선으로 쓸고 지나간 뒤 남색 바탕에 큰 금색 번호 `03`과 챕터명 "맵 편집기"
- duration: 2.5s
- transition_in: cut
- status: animated
- src: compositions/frames/01-chapter.html
- ref: R6 챕터 전환

## Frame 2 — 개념 카드

- scene: 같은 장면의 두 레이어. 위는 배경 그림, 아래는 빨간 충돌 마스크. 둘이 겹쳐지며 "보이는 그림과 밟는 바닥은 다른 데이터"
- duration: 5.5s
- transition_in: crossfade
- status: animated
- src: compositions/frames/02-concept.html
- ref: R8 개념 카드, R6 강조 카드
- caption: "배경 그림과 충돌 판정은 서로 다른 PNG입니다"

## Frame 3 — 편집 실화면

- scene: Ground Mask 편집기 녹화를 둥근 카드에 넣고, 좌상단 장면 제목 "Ground Mask 편집기", 우상단 챕터 배지. 사각형으로 칠하고 선으로 경사를 긋는 장면
- duration: 12s
- transition_in: crossfade
- status: animated
- src: compositions/frames/03-editor.html
- media: Source3 240–252초, 크롭 x=380 y=140 980×720
- ref: R6 자료 카드, R4 자막
- caption: "충돌은 마스크를 직접 칠해서 고칩니다" → "사각형·선·채우기, 코드는 건드리지 않습니다"

## Frame 4 — 게임 결과

- scene: 사막 경사를 달리는 플레이 화면을 화면 가득. 우상단 작은 배지만
- duration: 7s
- transition_in: cut
- status: animated
- src: compositions/frames/04-result.html
- media: Source3 약 849–856초
- ref: R7 플레이
- caption: "칠한 빨간 영역이 그대로 캐릭터가 밟는 바닥이 됩니다"

## Frame 5 — 결론 카드

- scene: 남색 바탕 좌측 정렬 큰 제목 2줄. "1px 수정에 / **빌드가 필요 없다**" 마지막 단어 노란색. 위에 회색 라벨 "데이터로 분리한 결과"
- duration: 3s
- transition_in: crossfade
- status: animated
- src: compositions/frames/05-outro.html
- ref: R6 강조 카드
