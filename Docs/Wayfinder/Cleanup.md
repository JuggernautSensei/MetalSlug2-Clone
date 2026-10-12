# 미사용 리소스 정리

모두 삭제가 아니라 `Trash/Unused/` 로 이동 (원래 상대 경로 유지). 복구용 JSON 도 같은 폴더.

| 항목 | 이전 | 이후 | 제거 |
|---|---|---|---|
| anim.json 클립 | 928 | 698 | 230 (`Trash/Unused/anim_removed.json` 에 본문 보존) |
| sprites.json 항목 | 230 | 173 | 57 (`sprites_sounds_removed.json`) |
| Textures/*.png | 15 | 6 | 9 (DevBackground, devBackgroundPixel, Stage0_Pixel_0/1, water, Enemy_Chaser_Bullet, MissleTank_Proj, Aparch_Turn_L/R) |
| Audio + sounds.json | 24 | 21 | 3 (Sfx_Yah, Sfx_YeeHa, Title_MS3) |
| 아틀라스 페이지 | 16 | 16 | 0 (참조 없는 페이지 없음, 재패킹 안 함) |

## 판정 기준
- 참조 = Source/**/*.cpp,*.h 의 문자열 리터럴 + Defs/Stage/Tuning JSON 문자열. `{side}`/`{W}`/`{S}` 는 패턴으로 확장, `_Left`/`_Right` 접미 허용.
- 접두 리터럴(`...Prefix_`)과 코드가 번호를 붙여 만드는 이름은 접두로 보존: Boss_Turret_, Boss_Cannon_Shot, Heli_Rotor_, Slug_Vulcan_Shot_, Slug_Shell_, Slug_CannonFx_, Bus_BackD/BodyD, Fx_Blood_Hit_/Splat_, ShotGun_Bullet_, HM_Bullet_, Wood/Rock/Metal N.

## 불확실해서 남긴 것
- 테스트만 참조하는 클립 9종 (Order_Boy_Run/Recoil/Stopping/Turn, Parker_Run/Stopping, FlameShot_Orig_*) 은 테스트 보호 때문에 유지.
- UiTest 가 요구하는 PressStartUI 스프라이트는 복구해서 유지.

## 알려진 기존 실패 (이번 작업과 무관)
- AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor
- StageDataReal.Stage0LoadsWithExpectedCounts (Stage0.json 미커밋 수정, 198 vs 199) 및 Regress stage0_walk 해시 (원본 anim 으로 되돌려도 동일하게 f0ec6cf24cf3dafb).
