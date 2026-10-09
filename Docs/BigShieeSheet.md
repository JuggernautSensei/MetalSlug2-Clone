# Big Shiee boss sheet (40204.png)

Source: `Assets/Source/40204.png` (4624x3317). Key: `color` (153,217,234) and (0,255,0) (green block behind the main body and treads), tol 0.
Entry: `Assets/Source/sheets.json` (object with `"file": "40204.png"`). All clips use `pivot: center`, `fps: 12`, explicit `frames` rects (alpha-trimmed by the baker).
Coordinates below are sheet pixels (top-left origin). `loop` is the baked loop flag. Labels in the sheet sit ABOVE their group (for turrets and body columns).

Baked: 124 clips, 1195 frames (`Scripts/Bake.ps1` exit 0). No empty frames.

## Clips

| Clip | Frames | Loop | Region (x0,y0-x1,y1) | Notes |
|---|---|---|---|---|
| `BigShiee_Body_Idle` | 8 | yes | 5,27-276,1726 | Body grid col c0 (label BIG SHIEE IDLE). Frames r0..r7. |
| `BigShiee_Body_DriveForward` | 8 | yes | 286,27-557,1726 | Body grid col c1 (DRIVING FOWARD [sic]). |
| `BigShiee_Body_DriveBack` | 8 | yes | 567,27-838,1726 | Body grid col c2 (DRIVING BACKWARDS). |
| `BigShiee_Cannon_Deploy1` | 8 | no | 848,27-1119,1726 | Body grid deploy stage (label DEPLOYING CANNON (1)). |
| `BigShiee_Cannon_Deploy2_Unused` | 8 | no | 1129,27-1403,1726 | Body grid deploy stage (UNUSED in sheet). |
| `BigShiee_Cannon_Deploy3` | 8 | no | 1413,27-1691,1726 | Body grid deploy stage (label DEPLOYING CANNON (3)). |
| `BigShiee_Cannon_Deploy4_Unused` | 8 | no | 1701,27-1983,1726 | Body grid deploy stage (UNUSED in sheet). |
| `BigShiee_Cannon_Deploy5_Unused` | 8 | no | 1993,27-2279,1726 | Body grid deploy stage (UNUSED in sheet). |
| `BigShiee_Cannon_Deploy6` | 8 | no | 2289,27-2579,1726 | Body grid deploy stage (label DEPLOYING CANNON (6)). |
| `BigShiee_Cannon_Deploy7_Unused` | 8 | no | 2589,27-2883,1726 | Body grid deploy stage (UNUSED in sheet). |
| `BigShiee_Cannon_Idle` | 8 | yes | 2893,27-3191,1726 | Body grid col c10 (CANNON DEPLOYED IDLE). Sheet label line 2 also reads DEPLOYING CANNON (8): no separate frames found. |
| `BigShiee_Cannon_DriveForward` | 8 | yes | 3201,27-3499,1726 | Body grid col c11 (CANNON DEPLOYED DRIVING FOWARD [sic]). |
| `BigShiee_Cannon_DriveBack` | 8 | yes | 3509,27-3807,1726 | Body grid col c12 (CANNON DEPLOYED DRIVING BACKWARDS). |
| `BigShiee_Cannon_Deploy9` | 8 | no | 3817,27-4117,1726 | Body grid deploy stage (label DEPLOYING CANNON (9)). |
| `BigShiee_Turret_A_Idle` | 22 | yes | 5,1753-1024,1789 | Turret A, IDLE group, row 1 (label TURRET A IDLE above). |
| `BigShiee_Turret_A_Idle_2` | 22 | yes | 5,1794-704,1829 | Turret A, IDLE group, row 2 (label TURRET A IDLE above). |
| `BigShiee_Turret_A_Idle_3` | 22 | yes | 5,1834-1024,1870 | Turret A, IDLE group, row 3 (label TURRET A IDLE above). |
| `BigShiee_Turret_A_Turning` | 9 | yes | 5,1888-352,1922 | Turret A, TURNING group (label above). |
| `BigShiee_Turret_A_Shooting` | 14 | no | 5,1940-595,1981 | Turret A, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_A_Shooting_2` | 14 | no | 5,1986-448,2024 | Turret A, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_A_Shooting_3` | 14 | no | 5,2029-595,2070 | Turret A, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_A_Destroyed` | 1 | no | 5,2088-68,2129 | Turret A, DESTROYED group (label above; single wreck frame). |
| `BigShiee_Turret_B_Idle` | 22 | yes | 5,2156-914,2189 | Turret B, IDLE group, row 1 (label TURRET B IDLE above). |
| `BigShiee_Turret_B_Idle_2` | 22 | yes | 5,2194-704,2229 | Turret B, IDLE group, row 2 (label TURRET B IDLE above). |
| `BigShiee_Turret_B_Idle_3` | 22 | yes | 5,2234-937,2267 | Turret B, IDLE group, row 3 (label TURRET B IDLE above). |
| `BigShiee_Turret_B_Turning` | 9 | yes | 5,2285-330,2319 | Turret B, TURNING group (label above). |
| `BigShiee_Turret_B_Shooting` | 14 | no | 5,2337-521,2371 | Turret B, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_B_Shooting_2` | 14 | no | 5,2376-443,2414 | Turret B, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_B_Shooting_3` | 14 | no | 5,2419-570,2460 | Turret B, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_B_Destroyed` | 1 | no | 5,2478-44,2520 | Turret B, DESTROYED group (label above; single wreck frame). |
| `BigShiee_Turret_C_Idle` | 22 | yes | 5,2547-1002,2583 | Turret C, IDLE group, row 1 (label TURRET C IDLE above). |
| `BigShiee_Turret_C_Idle_2` | 22 | yes | 5,2588-704,2623 | Turret C, IDLE group, row 2 (label TURRET C IDLE above). |
| `BigShiee_Turret_C_Idle_3` | 22 | yes | 5,2628-956,2664 | Turret C, IDLE group, row 3 (label TURRET C IDLE above). |
| `BigShiee_Turret_C_Turning` | 9 | yes | 5,2682-349,2716 | Turret C, TURNING group (label above). |
| `BigShiee_Turret_C_Shooting` | 14 | no | 5,2734-586,2775 | Turret C, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_C_Shooting_2` | 14 | no | 5,2780-448,2818 | Turret C, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_C_Shooting_3` | 14 | no | 5,2823-584,2864 | Turret C, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_C_Destroyed` | 2 | no | 5,2882-67,2911 | Turret C, DESTROYED group (label above; single wreck frame). Two pieces. |
| `BigShiee_Turret_D_Idle` | 22 | yes | 5,2938-1024,2972 | Turret D, IDLE group, row 1 (label TURRET D IDLE above). |
| `BigShiee_Turret_D_Idle_2` | 22 | yes | 5,2977-704,3012 | Turret D, IDLE group, row 2 (label TURRET D IDLE above). |
| `BigShiee_Turret_D_Idle_3` | 22 | yes | 5,3017-1024,3053 | Turret D, IDLE group, row 3 (label TURRET D IDLE above). |
| `BigShiee_Turret_D_Turning` | 9 | yes | 5,3074-352,3108 | Turret D, TURNING group (label above). |
| `BigShiee_Turret_D_Shooting` | 14 | no | 5,3126-595,3162 | Turret D, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_D_Shooting_2` | 14 | no | 5,3167-448,3205 | Turret D, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_D_Shooting_3` | 14 | no | 5,3210-595,3251 | Turret D, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_D_Destroyed` | 1 | no | 5,3269-45,3307 | Turret D, DESTROYED group (label above; single wreck frame). |
| `BigShiee_Turret_E_Idle` | 12 | yes | 1044,1753-1566,1778 | Turret E, IDLE group, row 1 (label TURRET E IDLE above). |
| `BigShiee_Turret_E_Idle_2` | 12 | yes | 1044,1783-1423,1813 | Turret E, IDLE group, row 2 (label TURRET E IDLE above). |
| `BigShiee_Turret_E_Idle_3` | 12 | yes | 1044,1818-1568,1843 | Turret E, IDLE group, row 3 (label TURRET E IDLE above). |
| `BigShiee_Turret_E_Turning` | 9 | yes | 1044,1861-1369,1887 | Turret E, TURNING group (label above). |
| `BigShiee_Turret_E_Shooting` | 11 | no | 1044,1905-1510,1936 | Turret E, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_E_Shooting_2` | 11 | no | 1044,1941-1391,1970 | Turret E, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_E_Shooting_3` | 11 | no | 1044,1975-1510,2006 | Turret E, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_E_Destroyed` | 1 | no | 1044,2024-1078,2061 | Turret E, DESTROYED group (label above; single wreck frame). |
| `BigShiee_Turret_F_Idle` | 12 | yes | 1044,2088-1568,2117 | Turret F, IDLE group, row 1 (label TURRET F IDLE above). |
| `BigShiee_Turret_F_Idle_2` | 12 | yes | 1044,2122-1424,2154 | Turret F, IDLE group, row 2 (label TURRET F IDLE above). |
| `BigShiee_Turret_F_Idle_3` | 12 | yes | 1044,2160-1568,2189 | Turret F, IDLE group, row 3 (label TURRET F IDLE above). |
| `BigShiee_Turret_F_Turning` | 9 | yes | 1044,2207-1370,2237 | Turret F, TURNING group (label above). |
| `BigShiee_Turret_F_Shooting` | 11 | no | 1044,2255-1505,2290 | Turret F, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_F_Shooting_2` | 11 | no | 1044,2295-1391,2326 | Turret F, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_F_Shooting_3` | 11 | no | 1044,2331-1505,2366 | Turret F, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_F_Destroyed` | 1 | no | 1044,2384-1078,2409 | Turret F, DESTROYED group (label above; single wreck frame). |
| `BigShiee_Turret_G_Idle` | 12 | yes | 1044,2436-1568,2463 | Turret G, IDLE group, row 1 (label TURRET G IDLE above). |
| `BigShiee_Turret_G_Idle_2` | 12 | yes | 1044,2468-1423,2500 | Turret G, IDLE group, row 2 (label TURRET G IDLE above). |
| `BigShiee_Turret_G_Idle_3` | 12 | yes | 1044,2505-1568,2532 | Turret G, IDLE group, row 3 (label TURRET G IDLE above). |
| `BigShiee_Turret_G_Turning` | 9 | yes | 1044,2550-1371,2578 | Turret G, TURNING group (label above). |
| `BigShiee_Turret_G_Shooting` | 11 | no | 1044,2596-1510,2629 | Turret G, SHOOTING group, row 1 (label above). |
| `BigShiee_Turret_G_Shooting_2` | 11 | no | 1044,2634-1391,2665 | Turret G, SHOOTING group, row 2 (label above). |
| `BigShiee_Turret_G_Shooting_3` | 11 | no | 1044,2670-1510,2703 | Turret G, SHOOTING group, row 3 (label above). |
| `BigShiee_Turret_G_Destroyed` | 1 | no | 1044,2721-1080,2748 | Turret G, DESTROYED group (label above; single wreck frame). |
| `BigShiee_Fire_ABCD_1` | 22 | no | 1588,2254-2395,2309 | Shared firing explosion row (label TURRET A,B,C & D). Bright then smoke tail. |
| `BigShiee_Fire_ABCD_2` | 22 | no | 1588,2319-2396,2374 | Shared firing explosion row (label TURRET A,B,C & D). Bright then smoke tail. |
| `BigShiee_Fire_ABCD_3` | 22 | no | 1588,2384-2392,2440 | Shared firing explosion row (label TURRET A,B,C & D). Bright then smoke tail. |
| `BigShiee_Fire_EFG_1` | 19 | no | 1588,2463-2169,2515 | Shared firing explosion row (label TURRET E,F & G). |
| `BigShiee_Fire_EFG_2` | 19 | no | 1588,2525-2164,2579 | Shared firing explosion row (label TURRET E,F & G). |
| `BigShiee_Fire_EFG_3` | 19 | no | 1588,2589-2166,2641 | Shared firing explosion row (label TURRET E,F & G). |
| `BigShiee_Projectile_Orange` | 4 | yes | 1592,2659-1626,2669 | Projectile legend, 4 growth sizes (4-10 px) of this colour. |
| `BigShiee_Projectile_Red` | 4 | yes | 1592,2671-1626,2681 | Projectile legend, 4 growth sizes (4-10 px) of this colour. |
| `BigShiee_Projectile_Cyan` | 4 | yes | 1592,2683-1626,2693 | Projectile legend, 4 growth sizes (4-10 px) of this colour. |
| `BigShiee_Antenna_1` | 19 | no | 1588,2718-2386,2757 | Antenna pieces, row A (under CANNON DEPLOYMENT / ANTENNAE). Piece grouping low confidence. |
| `BigShiee_Antenna_2` | 12 | no | 1588,2761-2103,2800 | Antenna pieces, row B. Piece grouping low confidence. |
| `BigShiee_Emblem_Spin_A` | 3 | yes | 1588,2817-1693,2851 | Emblem spin row (label EMBLEM). |
| `BigShiee_Emblem_Spin_B` | 8 | yes | 1588,2856-1903,2889 | Emblem spin row (label EMBLEM). |
| `BigShiee_Emblem_Ring` | 4 | no | 1586,2892-1737,2934 | Glow ring frames, grid cells (fixed rects). |
| `BigShiee_Spark` | 8 | no | 1588,2950-2276,3013 | Cyan spark blocks under SPARK label. Grouping low confidence. |
| `BigShiee_Gun_Aim` | 9 | yes | 2428,1910-3356,2060 | Gun-head grid row 0 (label AIMING). |
| `BigShiee_Gun_Shoot_1` | 8 | no | 2429,2066-3322,2220 | Gun-head grid row 1 (label SHOOTING on row 1; rows 2-4 unlabelled). |
| `BigShiee_Gun_Shoot_2` | 8 | no | 2429,2227-3269,2380 | Gun-head grid row 2 (label SHOOTING on row 1; rows 2-4 unlabelled). |
| `BigShiee_Gun_Shoot_3` | 8 | no | 2430,2388-3251,2540 | Gun-head grid row 3 (label SHOOTING on row 1; rows 2-4 unlabelled). |
| `BigShiee_Gun_Shoot_4` | 8 | no | 2430,2541-3238,2700 | Gun-head grid row 4 (label SHOOTING on row 1; rows 2-4 unlabelled). |
| `BigShiee_Gun_Shoot_5` | 8 | no | 2428,2701-3251,2866 | Gun-head grid row 5 (unlabelled). Merged pair split into fixed cells. |
| `BigShiee_Shell_Flame_1` | 12 | yes | 3658,1805-4207,1868 | Flaming shell row (12). |
| `BigShiee_Shell_Flame_2` | 12 | yes | 3658,1883-4096,1949 | Flaming shell row (12). |
| `BigShiee_Shell_Flame_3` | 12 | yes | 3658,1966-4057,2008 | Flaming shell row (12). |
| `BigShiee_Destroyed_Body` | 1 | no | 3658,2051-3957,2231 | Large destroyed body (label DESTROYED). |
| `BigShiee_Destroyed_Cannon` | 1 | no | 3962,2156-4010,2240 | Detached cannon piece next to destroyed body. |
| `BigShiee_Wreck` | 1 | no | 4334,2357-4605,2565 | Wreck, single frame. |
| `BigShiee_Body_Main_1` | 1 | no | 3659,2580-3781,2716 | Main body piece on green (0,255,0) field. |
| `BigShiee_Body_Main_2` | 1 | no | 3782,2580-3904,2716 | Main body piece on green (0,255,0) field. |
| `BigShiee_Body_Main_3` | 1 | no | 3905,2591-4036,2716 | Main body piece on green (0,255,0) field. |
| `BigShiee_Body_Main_4` | 1 | no | 4195,2627-4377,2791 | Main body piece on green (0,255,0) field. |
| `BigShiee_Body_Main_5` | 1 | no | 4378,2627-4560,2791 | Main body piece on green (0,255,0) field. |
| `BigShiee_Cannon_Pod` | 8 | yes | 3659,2717-4194,2791 | Row of 8 small cannon pods under MAIN BODY. |
| `BigShiee_Tread_1` | 4 | yes | 3659,2810-4430,2885 | Tread row (4 frames, 192x75). |
| `BigShiee_Tread_2` | 4 | yes | 3659,2886-4430,2961 | Tread row (4 frames, 192x75). |
| `BigShiee_Walker` | 3 | yes | 3320,1753-3638,1892 | Legged walker pieces, top right. Not verified as Big Shiee parts. |
| `BigShiee_Dome` | 2 | yes | 3658,2359-3843,2441 | Two large orange domes. |
| `BigShiee_Smoke_Puff` | 3 | no | 4069,2400-4200,2441 | Smoke puff growth (3). |
| `BigShiee_Fx_Misc_1` | 1 | no | 3848,2413-3871,2441 | Unidentified small effect piece. |
| `BigShiee_Fx_Misc_2` | 1 | no | 3876,2380-3907,2441 | Unidentified small effect piece. |
| `BigShiee_Fx_Misc_3` | 1 | no | 3909,2421-3937,2439 | Unidentified small effect piece. |
| `BigShiee_Fx_Misc_4` | 1 | no | 3948,2413-3971,2441 | Unidentified small effect piece. |
| `BigShiee_Fx_Misc_5` | 1 | no | 4005,2413-4065,2438 | Unidentified small effect piece. |
| `BigShiee_Fx_Strip_1` | 1 | no | 4083,2295-4124,2318 | Small effect sprite in the fx strip. Unidentified. |
| `BigShiee_Fx_Strip_2` | 1 | no | 4161,2292-4186,2312 | Small effect sprite in the fx strip. Unidentified. |
| `BigShiee_Fx_Strip_3` | 1 | no | 4195,2283-4244,2318 | Small effect sprite in the fx strip. Unidentified. |
| `BigShiee_Fx_Strip_4` | 1 | no | 4250,2289-4283,2318 | Small effect sprite in the fx strip. Unidentified. |
| `BigShiee_Smoke` | 18 | no | 2428,3107-3320,3155 | Smoke/dust row (18). |
| `BigShiee_Debris_Ball` | 2 | no | 2428,3219-2497,3250 | Debris balls (brown, white). |
| `BigShiee_Debris_Dust` | 24 | no | 2502,3200-3309,3248 | Dust dissipation sequence. |
| `BigShiee_Shell_Cartridge` | 5 | no | 2464,3066-2630,3088 | Brass cartridge row. |
| `BigShiee_Shell_Casing_1` | 3 | no | 2428,2955-2513,2980 | Shell casing row (unlabelled). |
| `BigShiee_Shell_Casing_2` | 3 | no | 2428,2985-2507,3013 | Shell casing row (unlabelled). |
| `BigShiee_Shell_Casing_3` | 3 | no | 2428,3018-2498,3047 | Shell casing row (unlabelled). |

## Not emitted (not animations)
- Palette swatches under the PALETTES label and the UNUSED swatch: about x 3658-4020, y 2255-2310 (swatch tiles) and small bars at y 2330-2470.
- Reference art (not sprites): x 4120-4430, y 2030-2240; x 2225-2375, y 2525-2650; x 1190-1370, y 2785-2990; x 2125-2305, y 2990-3205; bottom scene x 3658-4619, y 3095-3307.
- Credit and label text: "Ambidextrous Vehicle..." block (x 3660-4455, y 2980-3050), RIPPED BY and CUSTOM EDITS text, all section labels.

## Unresolved / for the placement pass
- Body column c10 label has a second line "DEPLOYING CANNON (8)" with no separate frames; treated as Cannon_Idle.
- Turret rows: label placement convention assumed label-above. Turret DESTROYED groups are single wreck frames (not 12-14 frames as first guessed).
- Antenna, Spark, Shell_Casing, Fx_Misc and Fx_Strip are best-effort grouping of loose pieces; verify visually before relying on them.
- Walker (top right, 3 frames) may not belong to Big Shiee; check before placing.
- All pivots are `center` (placeholder). Real offsets come from the placement pass.
