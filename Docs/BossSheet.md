# Big Shiee rebuilt boss sheet (Boss_* clips)

Source: `Assets/Source/40204.png` (4624x3317, 1x). Second sheet entry for the same file in `Assets/Source/sheets.json` (`scale: 2`, key `color` (153,217,234) and (0,255,0), tol 0, `pivot: center` on every clip). The old `BigShiee_*` entry is left untouched; the old C++ still uses it.

Baked: `Scripts/Bake.ps1` exit 0. 105 `Boss_*` clips, 1127 frames. The baker trims each frame to alpha, so frame sizes vary per frame where content varies.

Loop/fps hints: idle/driving/tread/turning/projectile/shell/emblem/gun-aim/cannon-pod loop; deploy/shooting/fire/destroyed/wreck/smoke/debris one-shot. Shooting 14 fps, fire 18 fps, turret idle 10 fps, other 12 fps (not measured, hints only).

Region column = bounding box of all frame rects of the clip in 1x sheet px (x0,y0-x1,y1).

## Dropped from the old set
- Deploy stages 2, 4, 5, 7: labelled UNUSED in the sheet.
- `Boss_Walker`: not verified as Big Shiee parts.
- `Boss_Fx_Misc_1..5`, `Boss_Fx_Strip_1..4`: unidentified single pieces.
- `Boss_Shell_Cartridge`, `Boss_Shell_Casing_1..3`: unlabelled, not requested.
- `Boss_Spark`: frames include unkeyed blue-grey background blocks (not clean sprites). Not usable until re-cut.

## Verification
- Contact sheets of all 105 clips (1x, key applied) were checked visually: no label text inside frames, no legend swatches (old Smoke legend frames already removed), turret, fire, gun, shell, body and emblem rows in the expected order.
- Known weak clips: `Boss_Antenna_1/2` are fragmented pieces (grouping low confidence). `Boss_Smoke_Puff` frame 0 has a pale tint (kept).

## Clips
| Clip | Frames | Loop | fps | Region |
|---|---|---|---|---|
| `Boss_Body_Idle` | 8 | yes | 12 | 5,27-276,1726 |
| `Boss_Body_DriveForward` | 8 | yes | 12 | 286,27-557,1726 |
| `Boss_Body_DriveBack` | 8 | yes | 12 | 567,27-838,1726 |
| `Boss_Cannon_Deploy1` | 8 | no | 12 | 848,27-1119,1726 |
| `Boss_Cannon_Deploy3` | 8 | no | 12 | 1413,27-1691,1726 |
| `Boss_Cannon_Deploy6` | 8 | no | 12 | 2289,27-2579,1726 |
| `Boss_Cannon_Idle` | 8 | yes | 12 | 2893,27-3191,1726 |
| `Boss_Cannon_DriveForward` | 8 | yes | 12 | 3201,27-3499,1726 |
| `Boss_Cannon_DriveBack` | 8 | yes | 12 | 3509,27-3807,1726 |
| `Boss_Cannon_Deploy9` | 8 | no | 12 | 3817,27-4117,1726 |
| `Boss_Turret_A_Idle` | 22 | yes | 10 | 5,1753-1024,1789 |
| `Boss_Turret_A_Idle_2` | 22 | yes | 10 | 5,1794-704,1829 |
| `Boss_Turret_A_Idle_3` | 22 | yes | 10 | 5,1834-1024,1870 |
| `Boss_Turret_A_Turning` | 9 | yes | 12 | 5,1888-352,1922 |
| `Boss_Turret_A_Shooting` | 14 | no | 14 | 5,1940-595,1981 |
| `Boss_Turret_A_Shooting_2` | 14 | no | 14 | 5,1986-448,2024 |
| `Boss_Turret_A_Shooting_3` | 14 | no | 14 | 5,2029-595,2070 |
| `Boss_Turret_A_Destroyed` | 1 | no | 12 | 5,2088-68,2129 |
| `Boss_Turret_B_Idle` | 22 | yes | 10 | 5,2156-914,2189 |
| `Boss_Turret_B_Idle_2` | 22 | yes | 10 | 5,2194-704,2229 |
| `Boss_Turret_B_Idle_3` | 22 | yes | 10 | 5,2234-937,2267 |
| `Boss_Turret_B_Turning` | 9 | yes | 12 | 5,2285-330,2319 |
| `Boss_Turret_B_Shooting` | 14 | no | 14 | 5,2337-521,2371 |
| `Boss_Turret_B_Shooting_2` | 14 | no | 14 | 5,2376-443,2414 |
| `Boss_Turret_B_Shooting_3` | 14 | no | 14 | 5,2419-570,2460 |
| `Boss_Turret_B_Destroyed` | 1 | no | 12 | 5,2478-44,2520 |
| `Boss_Turret_C_Idle` | 22 | yes | 10 | 5,2547-1002,2583 |
| `Boss_Turret_C_Idle_2` | 22 | yes | 10 | 5,2588-704,2623 |
| `Boss_Turret_C_Idle_3` | 22 | yes | 10 | 5,2628-956,2664 |
| `Boss_Turret_C_Turning` | 9 | yes | 12 | 5,2682-349,2716 |
| `Boss_Turret_C_Shooting` | 14 | no | 14 | 5,2734-586,2775 |
| `Boss_Turret_C_Shooting_2` | 14 | no | 14 | 5,2780-448,2818 |
| `Boss_Turret_C_Shooting_3` | 14 | no | 14 | 5,2823-584,2864 |
| `Boss_Turret_C_Destroyed` | 2 | no | 12 | 5,2882-67,2911 |
| `Boss_Turret_D_Idle` | 22 | yes | 10 | 5,2938-1024,2972 |
| `Boss_Turret_D_Idle_2` | 22 | yes | 10 | 5,2977-704,3012 |
| `Boss_Turret_D_Idle_3` | 22 | yes | 10 | 5,3017-1024,3053 |
| `Boss_Turret_D_Turning` | 9 | yes | 12 | 5,3074-352,3108 |
| `Boss_Turret_D_Shooting` | 14 | no | 14 | 5,3126-595,3162 |
| `Boss_Turret_D_Shooting_2` | 14 | no | 14 | 5,3167-448,3205 |
| `Boss_Turret_D_Shooting_3` | 14 | no | 14 | 5,3210-595,3251 |
| `Boss_Turret_D_Destroyed` | 1 | no | 12 | 5,3269-45,3307 |
| `Boss_Turret_E_Idle` | 12 | yes | 10 | 1044,1753-1566,1778 |
| `Boss_Turret_E_Idle_2` | 12 | yes | 10 | 1044,1783-1423,1813 |
| `Boss_Turret_E_Idle_3` | 12 | yes | 10 | 1044,1818-1568,1843 |
| `Boss_Turret_E_Turning` | 9 | yes | 12 | 1044,1861-1369,1887 |
| `Boss_Turret_E_Shooting` | 11 | no | 14 | 1044,1905-1510,1936 |
| `Boss_Turret_E_Shooting_2` | 11 | no | 14 | 1044,1941-1391,1970 |
| `Boss_Turret_E_Shooting_3` | 11 | no | 14 | 1044,1975-1510,2006 |
| `Boss_Turret_E_Destroyed` | 1 | no | 12 | 1044,2024-1078,2061 |
| `Boss_Turret_F_Idle` | 12 | yes | 10 | 1044,2088-1568,2117 |
| `Boss_Turret_F_Idle_2` | 12 | yes | 10 | 1044,2122-1424,2154 |
| `Boss_Turret_F_Idle_3` | 12 | yes | 10 | 1044,2160-1568,2189 |
| `Boss_Turret_F_Turning` | 9 | yes | 12 | 1044,2207-1370,2237 |
| `Boss_Turret_F_Shooting` | 11 | no | 14 | 1044,2255-1505,2290 |
| `Boss_Turret_F_Shooting_2` | 11 | no | 14 | 1044,2295-1391,2326 |
| `Boss_Turret_F_Shooting_3` | 11 | no | 14 | 1044,2331-1505,2366 |
| `Boss_Turret_F_Destroyed` | 1 | no | 12 | 1044,2384-1078,2409 |
| `Boss_Turret_G_Idle` | 12 | yes | 10 | 1044,2436-1568,2463 |
| `Boss_Turret_G_Idle_2` | 12 | yes | 10 | 1044,2468-1423,2500 |
| `Boss_Turret_G_Idle_3` | 12 | yes | 10 | 1044,2505-1568,2532 |
| `Boss_Turret_G_Turning` | 9 | yes | 12 | 1044,2550-1371,2578 |
| `Boss_Turret_G_Shooting` | 11 | no | 14 | 1044,2596-1510,2629 |
| `Boss_Turret_G_Shooting_2` | 11 | no | 14 | 1044,2634-1391,2665 |
| `Boss_Turret_G_Shooting_3` | 11 | no | 14 | 1044,2670-1510,2703 |
| `Boss_Turret_G_Destroyed` | 1 | no | 12 | 1044,2721-1080,2748 |
| `Boss_Fire_ABCD_1` | 22 | no | 18 | 1588,2254-2395,2309 |
| `Boss_Fire_ABCD_2` | 22 | no | 18 | 1588,2319-2396,2374 |
| `Boss_Fire_ABCD_3` | 22 | no | 18 | 1588,2384-2392,2440 |
| `Boss_Fire_EFG_1` | 19 | no | 18 | 1588,2463-2169,2515 |
| `Boss_Fire_EFG_2` | 19 | no | 18 | 1588,2525-2164,2579 |
| `Boss_Fire_EFG_3` | 19 | no | 18 | 1588,2589-2166,2641 |
| `Boss_Projectile_Orange` | 4 | yes | 12 | 1592,2659-1626,2669 |
| `Boss_Projectile_Red` | 4 | yes | 12 | 1592,2671-1626,2681 |
| `Boss_Projectile_Cyan` | 4 | yes | 12 | 1592,2683-1626,2693 |
| `Boss_Antenna_1` | 19 | no | 12 | 1588,2718-2386,2757 |
| `Boss_Antenna_2` | 12 | no | 12 | 1588,2761-2103,2800 |
| `Boss_Emblem_Spin_A` | 3 | yes | 12 | 1588,2817-1693,2851 |
| `Boss_Emblem_Spin_B` | 8 | yes | 12 | 1588,2856-1903,2889 |
| `Boss_Emblem_Ring` | 4 | yes | 12 | 1586,2892-1737,2934 |
| `Boss_Gun_Aim` | 9 | yes | 12 | 2428,1910-3356,2048 |
| `Boss_Gun_Shoot_1` | 8 | no | 14 | 2429,2066-3322,2220 |
| `Boss_Gun_Shoot_2` | 8 | no | 14 | 2429,2227-3269,2380 |
| `Boss_Gun_Shoot_3` | 8 | no | 14 | 2430,2388-3251,2540 |
| `Boss_Gun_Shoot_4` | 8 | no | 14 | 2430,2541-3238,2700 |
| `Boss_Gun_Shoot_5` | 8 | no | 14 | 2428,2701-3251,2866 |
| `Boss_Shell_Flame_1` | 12 | yes | 12 | 3658,1805-4207,1868 |
| `Boss_Shell_Flame_2` | 12 | yes | 12 | 3658,1883-4096,1949 |
| `Boss_Shell_Flame_3` | 12 | yes | 12 | 3658,1966-4057,2008 |
| `Boss_Destroyed_Body` | 1 | no | 12 | 3658,2051-3957,2231 |
| `Boss_Destroyed_Cannon` | 1 | no | 12 | 3962,2156-4010,2240 |
| `Boss_Wreck` | 1 | no | 12 | 4334,2357-4605,2565 |
| `Boss_Body_Main_1` | 1 | no | 12 | 3659,2580-3781,2716 |
| `Boss_Body_Main_2` | 1 | no | 12 | 3782,2580-3904,2716 |
| `Boss_Body_Main_3` | 1 | no | 12 | 3905,2591-4036,2716 |
| `Boss_Body_Main_4` | 1 | no | 12 | 4195,2627-4377,2791 |
| `Boss_Body_Main_5` | 1 | no | 12 | 4378,2627-4560,2791 |
| `Boss_Cannon_Pod` | 8 | yes | 12 | 3659,2717-4194,2791 |
| `Boss_Tread_1` | 4 | yes | 12 | 3659,2810-4430,2885 |
| `Boss_Tread_2` | 4 | yes | 12 | 3659,2886-4430,2961 |
| `Boss_Dome` | 2 | yes | 12 | 3658,2359-3843,2441 |
| `Boss_Smoke_Puff` | 3 | no | 16 | 4069,2400-4200,2441 |
| `Boss_Smoke` | 16 | no | 16 | 2535,3107-3320,3155 |
| `Boss_Debris_Ball` | 2 | no | 16 | 2428,3219-2497,3250 |
| `Boss_Debris_Dust` | 24 | no | 16 | 2502,3200-3309,3248 |
