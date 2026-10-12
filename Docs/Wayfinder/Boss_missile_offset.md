# BigShiee 고폭탄 발사 위치

측정: 주포 Ready 후 정면 행 (row 2) 첫 고폭탄의 첫 위치 (선체 원점 기준), 값을 각각 +10 했을 때 (Test/BigShieeTest.cpp 임시 측정 하니스).

| 값 | 전 (이전 코드) | 후 |
|---|---|---|
| 기준 | (30, -117) | (30, -117) |
| weapons.missile.muzzle x/y | 변화 없음 (ExtraFromWorld_ 가 무기 muzzle 을 상쇄) | x +10 / y +10 그대로 따라간다 |
| params.missileMuzzle x | 변화 없음 (안 쓰임) | 파라미터 삭제 |
| params.missileMuzzle y | y +10 | 파라미터 삭제 |
| params.cannonMuzzleX[2] (정면 행) | x +10 | 변화 없음 (행 변화량은 가운데 행 대비) |
| params.cannonAnchor x | x +10 | 변화 없음 (주포 그림 위치 전용) |
| params.cannonAnchor y | 변화 없음 | 변화 없음 |
| params.cannonAimOrigin | 변화 없음 (조준 기준 전용) | 변화 없음 |

새 규칙: 발사점 = 선체 원점 + weapons.missile.muzzle (facing 으로 좌우 반전) + (cannonMuzzleX[행] - cannonMuzzleX[2]) * facing.
섬광/연기/탄피도 같은 최종 발사점 기준. 모션 창의 muzzle 마커 = 실제 포구.

기본값 주의: 지금 발사점은 (30, -117) 이다 (cannonAnchor.x 30 + cannonMuzzleX[2] 0, 사용자가 맞춘 missileMuzzle.y -117). 그래서 weapons.missile.muzzle = [30, -117] 로 넣어 위치는 그대로다. 머리 그림의 실제 포구는 선체 원점 위 약 -300 근처 (cannonAimOrigin y -301) 라 모션 창에서 마커를 올려 맞추면 된다.
