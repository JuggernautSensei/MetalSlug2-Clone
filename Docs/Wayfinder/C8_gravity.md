# C8 중력 특성 (physics.useGravity)

규칙: 지상형(보병/차량) = true, 비행체/보스/매달림 = false. 로더(`EnemyDefLoader.cpp` physics.useGravity -> `PhysicsDef::bGravity`)와
`EnemyWorld::Create_` (body.bUseGravity) 가 읽고, 모든 지상 행동은 `StepPhysics` 로 매 틱 물리를 돈다 (코드 변경 없음, 데이터만 명시).

| def | kind / behavior | before | after | reason |
|---|---|---|---|---|
| soldier_bomb/sword/missile (soldier_base) | 보병 / soldier | 암묵 true | 명시 true | 지상형. StepPhysics (Climb 제외) |
| soldier_bzk | 보병 / bzk | 암묵 true | 명시 true | 지상형. StepPhysics (Climb 제외) |
| shielder | 보병 / shielder | 암묵 true | 명시 true | 지상형. StepPhysics |
| pow | 인질 / pow | 암묵 true | 명시 true | 지상형. MoveDown 중에만 행동이 일시 끔 |
| jeep | 차량 / jeep | 암묵 true | 명시 true | 지상형. StepPhysics |
| btank | 차량 / btank | 물리 블록 없음 (기본 true) | 명시 true | 지상형. 비활성 때도 StepPhysics |
| missle_tank | 차량 / missle_tank | 물리 블록 없음 (기본 true) | 명시 true | 지상형. StepPhysics |
| sarubia | 차량 / sarubia | 암묵 true | 명시 true | 지상형. 비활성 때도 StepPhysics |
| soldier_hang | 보병 / hang | false | false | 천장에 매달림 (공중 고정). 사망 때만 공통 사망 물리로 낙하 |
| flying_bzk | 낙하산 / flying_bzk | false | false | 비행체. descendSpeed 등속 낙하 (자체 이동) |
| pow_chute | 낙하 인질 / flying_bzk | false (상속) | false | 자체 fallGravity 자유낙하 (건드리지 않음) |
| heli | 헬기 / heli | false | false | 비행체 (보스) |
| heli_bomb | 폭탄 / heli_bomb | false | false | 투사체형, 자체 gravity/maxFall (HeliBombCfg) |
| bigshiee | 보스 / bigshiee | false | false | 보스 |
| bus | 버스 중간보스 / bus | false | false | 정적 오브젝트형 보스 (물리 대상 아님) |

기본값 `PhysicsDef::bGravity = true` 는 유지 (로더가 키 생략을 허용). 사망 연출은 `bUseGravity = true` 로 모든 def 공통.
테스트: `Test/GravityTraitTest.cpp` (플래그 표 + 공중 80px 스폰 후 120틱 착지 / hang 은 y 불변).
