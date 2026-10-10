# Big Shiee final boss: reference spec from the original MS2 Mission 4 video

Source: `Assets/Source/videoplayback (2).mp4` (480x360, 29.5 fps, 378.1 s). Boss fight = t 250.0 to 378.1 s (video time).
Scope: observation only. No repo files were changed. Analysis artifacts (frame extractions, contact sheets, scripts) are in the scratch folder `spec_boss` (not in the repo).

Conventions
- t = video seconds. Frame refs `f2/NNNN.png` = 2 fps extraction, t = 250 + (NNNN-1)/2. Gray 10 fps array = t 250.0 + k/10.
- Coordinates: video px (480x360). Game px assumed = video px x 1.333 (x), x 1.244 (y) (full-frame stretch to 640x448). NOT verified (see Unknowns). Ratios hold regardless.
- Confidence: [H] measured and cross-checked, [M] measured once or visually clear, [L] inferred / noisy.
- Sign of scroll: negative dx = image content moves to -x on screen = camera pans to +x.

## 0. Corrections to the brief (video contradicts these)
- Skull cannon deploy starts t=299.2, fully up with golden head at t=300.8 (not 298-303) [H].
- Skull cannon destroyed t=341.0-343.2 with large flash. Hull breaks up t=346-350. Results overlay at t=350.5 (not 352-355) [H].
- Hull ENTERS FROM THE SCREEN LEFT (x<0) at t=257, not from the right. Engine `entranceFromX: 500` may be mirrored [M].
- Skull is already golden from deploy; no per-hit gold flash is visible at 10 fps. Hit flash not confirmed [M].
- Existing `Docs/BigShieeLayout.md` was calibrated on `videoplayback (1).mp4` (496x360, 388 s). Its scale (0.82, 0.80) and origin do NOT apply to this video [H].

## 1. Timeline (video seconds)
| t (s) | On screen | Refs | Conf |
|---|---|---|---|
| 250.0-253.0 | Fast leftward world motion (rubble mean -80 video px/s, peak -100). Explosion and smoke at left-centre. Cyan tracers from left. Small enemy vehicle blown up. | f2/0001-0007 | M |
| 253.0-256.0 | Smoke/debris clears. Small walkers and explosions at left-centre (t=256). | f2/0007-0013 | M |
| 257.0-258.5 | Hull enters from LEFT edge: turret/tower block at x 0-60 (257.0), blue fire at x 0-10 (257.5), hull fills x 0-100 (258.5). | 10fps entrance sheet | M |
| 259.0-262.0 | Hull fills centre-left; bow (round dome) appears right of centre by 262. Explosions at bottom-left. | f2/0017-0025 | M |
| 262.0-272.0 | World bursts left at ~12 video px/s. Hull drives right on screen ~+11 px/s (bow dome x 340 -> 455). Red shots, cyan tracers, big explosions top-centre (264-270). | 2fps sheets 262-273 | M |
| 272.0-281.0 | World still. Hull static on screen (bow ~455). Enemy soldiers at bottom-left (272-276). Slug "IN" prompts (276, 279, 282). | sheet cs2_02 | M |
| 282.0-286.0 | World burst left (-12.8 video px/s). Cyan volley fan (282-285), orange shells arc. | flow | M |
| 287.0-299.0 | World still. Large cyan volleys 288-291 (many streaks). Explosions top-left. Slug "IN" at 293, 300. | f2/0075-0099 | M |
| 299.2-300.8 | CANNON DEPLOY: grey mechanical head rises at top-right of hull (299.2), golden head by 300.8. | skull_deploy_a | H |
| 300.8-302.5 | Golden skull idle, aiming at left. Small fires at deck. | skull_deploy_b | M |
| 302.8-305.0 | First cannon volley (cyan streaks left-down); dark smoke puff at head (303.0); orange flame shells burst (303.5-305). White flash frame 305.4. | skull_deploy_b | M |
| 305.0-331.0 | Skull volleys every ~2.5-3.5 s (307, 310, 313, 316, 320, 325, 328, 331). Orange flame shells arc 305-320. Explosions bottom-left (316, 322, 329). | cs2_04-05 | M |
| 332.0-342.0 | World burst left (-15.8 video px/s). Skull volleys 333-341, smoke column rises from centre deck (339-340). White flash 336 (944 white px). | cs2_06-07, flow | M |
| 340.4-342.4 | Skull explosion sequence: flame at head (341.2), dark smoke cloud (341.6), head gone by 342.4. | death_a | M |
| 343.0 | Largest flash: flame column + white disc (1694 white px). | death_a 343.0 | H |
| 343.5-347.0 | Chain blasts across hull: black smoke ball 344.0, white discs 344.4-346.8, orange blasts. Body still standing. | death_a | M |
| 348.0-350.0 | Hull splits, left part smoking, big orange flash at 350.0 (14478 orange px). | cs2_08 | M |
| 350.5 | RECAPTURED PRISONER overlay appears. Scene luma drops 98 -> 65. | cs2_08 | H |
| 351.0-368.0 | Results: prisoner count 01..16 at ~0.5 s each; score per table in section 7. | cs2_08-09 | H |
| 369.0-370.0 | Results cleared. Luma jumps 73 -> 123. | series | H |
| 370.0-376.0 | "MISSION 4 COMPLETE!" letters animate (370-374). Background: rubble, wreck pieces left. | cs2_10 | H |
| 377.0-378.1 | Fade to black (luma 43 at 377, 0 at 378). | cs2_10 | H |

## 2. World scroll
Method: Farneback dense flow between consecutive 10 fps frames, median per region. HUD top band (y 0-14) has ZERO flow for the whole fight, so there is no whole-frame shake or pan [H].

Rubble band (x 90-300, y 300-340) mean dx per 0.1 s:
| Window | Mean (video px/s) | Integral (video px) | Pattern |
|---|---|---|---|
| 250-254 | -80 | -320 | fast, explosion era, [L] |
| 255-259 | -10 | -39 | |
| 260-271 | -12.2 | -146 | steady -1.5 px per 0.1 s frame |
| 272-281 | 0 | -3 | still |
| 282-286 | -12.8 | -64 | steady |
| 287-331 | 0 | -17 | still |
| 332-342 | -15.8 | -174 | steady |
| 343-377 | 0 | -4 | still |
Total -766 video px = -1021 game px (x1.333) over 128 s [M].

Findings
- Camera is NOT locked and NOT continuously scrolling. It pans in discrete bursts (3 main bursts: 260-271, 282-286, 332-342; plus a fast start at 250-254) at ~12-16 video px/s (~16-21 game px/s), then stops [M].
- Pan direction: content moves -x, so the map slides to +x. This matches the brief's "slides to +x" [M].
- Relation to hull: during the three bursts the hull's lower body, treads and rubble move together, so the hull is effectively world-static and the camera does the moving [M]. Exception: 258-272, when the hull drives to +x on screen (~+11 video px/s) while the ground still moves -12 px/s, so the hull's world speed is ~+24 video px/s (~32 game px/s) during that window [L].
- Treads: their flow equals the ground's in bursts, so the tread animation is not separable from the scroll at 10 fps [M].
- Players: the players ride Slugs in the lower-left. The camera pan is not obviously tied to player position (not measured) [L].

## 3. Hull
- Composition: dark steel battleship. Bow at right with a grey round dome (bow port) and an orange/red boiler block below. Stern (left) with turret stacks. Treads at bottom. Stacked turret decks. Scaffold tower near the right edge at t=262.
- Size: bow-to-stern ~400 video px (~530 game px). Height from turret tops (y 40-60) to tread bottom (y ~300) ~250 video px (~310 game px). Matches engine `hullBox 560x330` only roughly [M].
- Screen position (dark-mask bow edge, p95): ~340 (262), ~455-470 (272-288), ~420 (292-330), ~330 (340), ~260 (345-348). Mean hull centre ~350-420 [L].
- Drive animation: treads visible on bottom band. Clip names not separable at 10 fps. Use BigShiee_Body_DriveForward/DriveBack (8 frames each, loop) and Tread_1/2 (4 frames, loop), tread fps unknown [L].
- Damage stages (visual, [M]): t 262-272 smoke at deck; 280-300 deck and turret smoke increasing; 300-340 smoke columns, skull cannon becomes main damage source; 343-350 hull split apart.
- Hit flash: NOT isolated. The only white flashes are big explosions (336, 343, 346: 944-1694 white px). Gold tint on the skull persists from deploy. RGB of the golden skull ~ (220,180,80) [L].
- Shake: none detected (HUD flow 0) [H].

## 4. Turrets (A-G)
Mapping of visible turrets to A-G is NOT established. Visible groups (visual, [L]):
- Top-left cluster with spiky domes (likely A/G).
- Top-centre cluster with yellow/black striped structure (likely B).
- Right-centre bow turrets and round domes (likely E/F).
Positions relative to origin: use the engine's current values only as a placeholder (A (-130,-108), B (-8,-146), C (52,-33), D (-48,-28), E (150,-14), F (85,24), G (-76,-90) in game px, from `Docs/BigShieeLayout.md`). Not verified against this video.

Cadence and shots (visual, [M])
- Red teardrop shots: 262-272, 282-286, 292-296. Roughly one burst every ~4 s.
- Cyan tracers (blue dashes): 264-270, 277-282, 288-291 (heavy), 302-341 (cannon).
- Orange flame shells: arc 282-287, 305-320, 340-345 (cannon).
- Projectile speed: NOT measured (blob tracking failed). Engine uses tracer 230 px/s, shell flight 1.0 s.
- Destroyed turrets: visible explosions and wrecks top-left at 264-270, 285-291, 297-300, 333-337. Count of wrecks: unknown (estimate 4-6) [L].
- Hit points: unknown. No per-hit evidence at 10 fps.
- Destroyed animation: explosion (Fire_ABCD / Fire_EFG rows) then wreck frame. Visible in cs2_04-06.

## 5. Main cannon ("skull")
- Deploy: 299.2 (start rise) -> 300.8 (golden head up). Duration ~1.6 s [H]. Sheet has 9 deploy stages (Deploy1..9, 2/4/5/7 unused); the video shows a deploy of ~6-8 stages in 1.6 s. Stage-to-time mapping is [L].
- Idle/aim: 300.8-302.5 golden head aims left, Gun_Aim frames [M].
- Volleys: 302.8, ~305, ~307, ~310, ~313, ~316, ~320, ~325, ~328, ~331, ~333, ~336, ~339 [M]. Interval ~2.5-3.5 s (engine 4-5 s). Each volley 3-6 cyan streaks fanning left-down, streak spacing ~0.25 s [M].
- Recoil: not clearly measurable.
- Shell: orange flame shells arc after every 2nd volley (matches engine `shellEvery: 2`) [M].
- Destruction: 340.4 flame at base, 341.2 head explosion, 341.6 smoke, 342.4 head gone. Large flash 343.0 [H].
- Post-destruction: smoke columns and chain blasts until 350; the head is not visible after 343 [M].

## 6. Other enemies and events
- Enemy soldiers at bottom-left (272-282), Slug "IN" prompts (276, 279, 282, 293, 300). Players ride Slugs at bottom-left.
- Debris and dark flecks falling: 252-262, 290-300, 340-348 (fragments).
- Explosions on rubble bottom-left: 262-268, 296-299, 316-330, 336-345.
- Planes, mortars, POW parachutes, HMG/bomb drops: NOT observed in this fight.
- Player damage events: not identified. HUD lives stay at 2 (1UP=2) throughout. HUD weapon/ammo changes across the fight (CANNON/BOMB labels) [L].
- Dodge positions: not measured.

## 7. Death sequence (t 336-378)
| t | Event | Conf |
|---|---|---|
| 336.0 | White flash (944 px) | M |
| 338.0-340.0 | Blue streaks from skull; smoke column from centre deck | M |
| 340.4-341.6 | Skull flame, dark smoke, head gone | M |
| 343.0 | Flame column + large white disc (1694 px) | H |
| 344.0-346.8 | Black smoke ball 344.0; white discs 344.4-346.8; orange chain blasts | M |
| 347.0-349.0 | Hull still partly standing, smoke | M |
| 350.0 | Big orange flash (14478 px). Hull left part visible until ~350 | M |
| 350.5 | RECAPTURED PRISONER overlay. Luma 98 -> 65 | H |
| 351.0-368.0 | Results (see below). Luma 65-78 | H |
| 369.0 | Results cleared. Luma 123 | H |
| 370.0-376.0 | MISSION 4 COMPLETE! letters animate. Luma 121-130 | H |
| 377.0-378.1 | Fade to black | H |

Results values (video time): 1P 00 (350.5), 01 10000 (351.0), 02 20000 (351.5), 03 30000 (352.0), 04 40000 (352.5), 05 50000 (353.0), 06 60000 (353.5), 07 70000 (354.0), 08 80000 (354.5), 09 90000 (355.0), 10 100000 (355.5), 11 110000 (356.0), 12 120000 (357.0), 13 130000 (357.5), 14 140000 (358.0), 15 150000 (358.5), bonus +30000 = 180000 (359.0), 16 prisoners 280000 (363.5), GREAT! 380000 (366.5).

Camera shake in death: none detected (HUD flow 0).

## 8. Sprite map (40204.png, sheet px, scale 1x; from `Docs/BigShieeSheet.md`)
Clip frames/loop from baked anim.json. fps: all clips 12 (baker default). Pivot: center (placeholder).
| Piece | Sheet region (x0,y0-x1,y1) | Frames | Loop | Use | z |
|---|---|---|---|---|---|
| Body_Idle | 5,27-276,1726 | 8 | yes | hull idle (column c0) | 1 |
| Body_DriveForward | 286,27-557,1726 | 8 | yes | drive fwd | 1 |
| Body_DriveBack | 567,27-838,1726 | 8 | yes | drive back | 1 |
| Cannon_Deploy1 | 848,27-1119,1726 | 8 | no | deploy stage 1 | 1 |
| Cannon_Deploy3 | 1413,27-1691,1726 | 8 | no | deploy stage 3 | 1 |
| Cannon_Deploy6 | 2289,27-2579,1726 | 8 | no | deploy stage 6 | 1 |
| Cannon_Deploy9 | 3817,27-4117,1726 | 8 | no | deploy stage 9 | 1 |
| Cannon_Idle | 2893,27-3191,1726 | 8 | yes | golden head deployed | 1 |
| Cannon_DriveForward/Back | 3201/3509,27-3807,1726 | 8 | yes | cannon drive | 1 |
| Turret_A_Idle (rows 1-3) | 5,1753-1024,1870 | 22 | yes | turret idle | 2 |
| Turret_A_Turning | 5,1888-352,1922 | 9 | yes | turn | 2 |
| Turret_A_Shooting (rows 1-3) | 5,1940-595,2070 | 14 | no | fire | 2 |
| Turret_A_Destroyed | 5,2088-68,2129 | 1 | no | wreck | 2 |
| Turret_B..G (same pattern) | rows 2156-3307 (x 5-1568) | 9-22 | | | 2 |
| Fire_ABCD_1/2/3 | 1588,2254-2396,2440 | 22 | no | firing flash, A-D | 3 |
| Fire_EFG_1/2/3 | 1588,2463-2169,2641 | 19 | no | firing flash, E-G | 3 |
| Projectile_Orange/Red/Cyan | 1592,2659-1626,2693 | 4 each | yes | shot sizes (4-10 px) | 4 |
| Gun_Aim | 2428,1910-3356,2060 | 9 | yes | cannon aim | 2 |
| Gun_Shoot_1..5 | 2428-3251,2066-2866 | 8 each | no | cannon fire rows | 2 |
| Shell_Flame_1/2/3 | 3658,1805-4207,2008 | 12 each | yes | orange shells | 4 |
| Destroyed_Body / Destroyed_Cannon | 3658,2051-3957,2231 / 3962,2156-4010,2240 | 1 | no | death | 1 |
| Wreck | 4334,2357-4605,2565 | 1 | no | final wreck | 0 |
| Body_Main_1..5 | 3659-4560,2580-2791 | 1 each | no | body pieces | 1 |
| Cannon_Pod | 3659,2717-4194,2791 | 8 | yes | pods under body | 1 |
| Tread_1 / Tread_2 | 3659,2810-4430,2961 | 4 each | yes | treads | 1 |
| Dome | 3658,2359-3843,2441 | 2 | yes | orange domes | 1 |
| Smoke | 2428,3107-3320,3155 | 18 | no | smoke row | 5 |
| Smoke_Puff | 4069,2400-4200,2441 | 3 | no | smoke growth | 5 |
| Debris_Dust | 2502,3200-3309,3248 | 24 | no | dust | 5 |
| Emblem_Spin_A/B | 1588,2817-1903,2889 | 3 / 8 | yes | X emblem spin | 1 |

## 9. Implementation outline (engine-consistent)
State machine (video-consistent):
1. ENTER (t 257-262 in video): hull enters from LEFT, drives to +x. Currently engine: 2.5 s from x 500 (verify mirror).
2. ASSAULT: turrets A-G fire in pairs (current engine: 12 s, 2.2 s bursts, 2 turrets). Camera bursts left/right at ~16 game px/s between bursts.
3. DEPLOY: ~1.6 s rise (engine: 4.0 s). Golden head at end.
4. CANNON: volleys every 2.5-3.5 s (engine 4-5 s), 3-6 streaks per volley, every 2nd volley a shell.
5. DESTROY: skull explosion sequence ~3 s (341-343), chain blasts ~7 s (343-350), hull split at 348-350.
6. RESULTS: overlay at +0.5 s after destroy, prisoner count 1 per 0.5 s.

Parameter table (video vs current `Assets/Defs/Boss.json`)
| Param | Video | Current | Action |
|---|---|---|---|
| Entrance side | left, 257-262 | from x 500 (right) | verify, possibly mirror |
| Entrance duration | ~5 s (257-262) | 2.5 s | adjust |
| Cannon deploy | 1.6 s | 4.0 s (deploySec) | shorten |
| Volley interval | 2.5-3.5 s | 4-5 s | shorten |
| Streaks per volley | 3-6 | 3 | verify |
| Shell every | 2nd volley | 2 | ok |
| Camera scroll | bursts ~16 game px/s, total ~1000 game px | not in data | add |
| Hull HP | not measured | hp 1300 (note says 900) | fix note mismatch |
| Destroy chain | ~9 s to split | 8 s chain + 10 s finish | ok |
| Results | 350.5 start | n/a | match |

Scroll mechanism: a `CameraPan` component with bursts defined by (start, end, speed) in game px/s, driven by the state machine, not by player position. Hull stays world-static during bursts; the drive only moves the hull during ENTER.

## 10. Unknowns
- Game px scale: full-frame stretch (x 1.333, y 1.244) assumed; not verified against our 640x448 HUD. Affects all game-px numbers.
- Turret A-G mapping and exact turret wreck count.
- Projectile speeds (blob tracking failed). Tracer 230 px/s used as engine reference only.
- Per-hit flash (none isolated at 10 fps; needs 29.5 fps check).
- Animation fps for each clip (10 fps sampling cannot resolve it).
- Hull x over time after 272 is noisy; treat screen positions as [L].
- Player positions and damage events.
- Whether the 250-254 fast pan is part of the boss fight or a stage transition.
- Mirror question: whether the hull's entrance side and bow side should be flipped relative to the current engine.

## 11. Verification notes
- 2 fps contact sheets: `cs2_00..cs2_10` (scratch). 10 fps colour and gray arrays (scratch). Flow and phase correlation scripts (scratch).
- No repo code changed. No build. No git.
