# 포트폴리오 영상 초안

`index.html`은 생성 파일이다. 고칠 곳은 두 군데뿐이다.

- `script.json` — 장면 순서, 원본 구간(in/out), 크롭, 자막. 자막은 `[장면 안 시작초, 길이, "문장"]`.
- `build/template.html` — 색(`--accent`), 폰트, 자막 크기 같은 스타일.

```bash
python build.py
npx --yes hyperframes@0.8.155 check
npx --yes hyperframes@0.8.155 render
```

- `build.py`는 `assets/clips/`에 없는 클립만 원본에서 새로 자른다. in/out이나 크롭을 바꾸면 해당 파일(`ed_<in>.mp4`, `play_<src>_<in>.mp4`, `trailer.mp4`)을 지우고 다시 실행한다.
- 원본 녹화는 `script.json`의 `sources` 경로에서 읽기만 한다.
- 스타일 기준과 레퍼런스: `../REFERENCES.md`. 기능 설명의 근거: `../../Docs/CodeStudy/06_Video_Claims.md`.
- 폰트는 테스트용 맑은 고딕(`assets/fonts`). 공개 배포 전에 Pretendard 같은 배포 가능한 폰트로 바꾼다.
