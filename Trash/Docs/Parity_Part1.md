# Parity Part 1: Original Mission 4, t = 0-130 s vs Stage0 (ours)

Scope: original footage t = 0-130 s (street, shops, trucks, bus event, bridge, wagon). Analysis only, no repo edits.

## 0. Method and calibration

- Source: `Assets/Source/videoplayback (2).mp4` (480x360, 29.5 fps). Frames sampled at 1 fps (t0-130), 6 contact sheets (scratch `par1`). Times are +/-1-2 s; enemy counts are visual estimates.
- Camera scroll: phase correlation on consecutive 1 s frames (bottom band), cumulated.
- Scale: world X ~= 1.26 x cumulative video scroll. Template matching of original frames against `Assets/Baked/Textures/Stage0_Main.png` gave the same ratio at t13, t20, t66, t69 (corr 0.5-0.6).
- Anchors that agree: truck@580 at right edge at t0; camera lock at X~2500 at t70 (ours bus lock 2513); bus_end 3050 at ~t86; bridge 3300 at ~t88; bridge_down 3700 at ~t100; bridge_mid 4037 at ~t113.
- Original camera X (world): t13 ~740 | t20 ~990 | t30 ~1230 | t40 ~1550 | t50 ~1735 | t60 ~1990 | t70-81 locked ~2500 | t82-89 scrolls to ~3080 | t90-98 stop ~3400 | t101-111 stop ~3680 | t116-130 stop ~4170.
- HUD: timer starts at 60, counts down one per ~5-6 s, and resets to 60 at t70 and t81. Ammo shows infinite pistol at start, bombs 10. Score popups in the original: 100/300/500/1000/5000.

## 1. Segment table (original time vs our stage section)

| Original t (s) | World X | Original has | Ours has | Delta | Pri | Needed sheet |
|---|---|---|---|---|---|---|
| 0-13 (intro, 0-750) | 0-740 | MISSION 4 banner t1-3. Blue-clad soldier standing at X~380 (t0-10). Green pickup at X580 (right edge at t0). Second truck X850 with soldiers, explodes t9-11. Grenade-throwers X900-1060 (red dots t9-16). Blue down-arrow over a yellow figure t2-4 (X~480), purpose unknown. | Banner 2.5 s. truck@580, truck@850. anim_decor Parker@386 (background layer). soldier_bomb@900, @1060. Sword wave at 7 s. | Blue-clad palette and blue arrow marker not in ours (check whether Parker is the blue soldier). Parker is background layer; original soldier stands in play layer. Only 3 enemies. | M | 11279 (soldier strips), 11302 |
| 13-30 (street A, 750-1230) | 740-1230 | Green camo soldiers (sword and bomb) from t12, 2-4 on screen. Grenade arcs t9-16. Big blast with red blood cloud t18-19. Soldiers on shop roofs t14-23. Shop roof collapse t20-23. Popups 300 (t18), 1000 (t21, t26). Truck near X1430 with riders t14-29. | sword@1420, bomb@1590, bomb@1760, bzk@2110 (+ waves at 3.5 s and 9 s). shop_0@1274, shop_1@1489, truck@1430, POW@1325 y140. | Enemy density about 5x lower (ours ~6 in X750-2244 vs original ~25-40). No blood splash. POW@1325 not identified in original. Shop collapse needs HP 12 with bombs/explosions. | H | 11279, 11302, 32529.gif |
| 30-47 (street A/B, 1230-1800) | 1230-1800 | Rubble of destroyed shops spreads across X1500-1800 (t29-47). Soldiers climbing rubble. Green pickup with riders X1700-1800 (t30-36). Explosions t33-34, t38-41 (fire + smoke). Popups 300/500. | shop_2@1661, shop_3@1906 (intact until destroyed). truck@1810. POW@1542 y140. | Rubble is much larger and persistent in original (ours uses one destroyed sprite per shop). No rocket/fire from riding truck. | H | 11302, 53213, 39079 |
| 47-66 (street B, 1800-2240) | 1800-2240 | Enemy vehicle + 5+ soldiers on roofs (t48-53, X1850-2000). Explosions t54 (X~1950), t58-60 (X~2100). H weapon crate on ground X~2070 with BULLET caption (t56-57). Rubble mounds X2000-2250 (t60-68). Popups 500/1000. | shop_4@2224, shop_5@2513, truck@2200, POW@2000 y140, bzk@2110. No weapon crate in street. | Missing H (heavy machine gun) crate (only gunitem_mg in boss). Missing roof soldier group and vehicle. Missing large rubble piles. | H | 39079, 11279, 11302 |
| 66-70 (bus_create to bus_start, 2240-2560) | 2240-2500 | Bus (mb_truck) appears on right at t69 (X~2375). Camera locks at t70 (X~2500). | bus_create@2244 (ground silent). mb_truck@2995 (visible from camera ~2370). bus_start@2560, lock 2513-3153. | Timing matches. Bus starts from right edge at about the same point. | L | 33825 |
| 70-86 (bus fight, 2500-3080) | 2500-3080 | Camera locked t70-81. Bus flashes orange on hit. Explosions on bus t73-79, smoke, soldiers with bazooka at left. Bus wreck pile t84-86 with BULLET caption. Timer reset t70 and t81. Score 5000 near t87. | bus_dead gate unlocks camera. soldier_sword wave at bus_end (1.5 s). | Verify bus hit-flash, Bus_Smoke, Bus_Damaged_1..3 and Bus_Wreck clips are used (all exist in 33825 clip map). Verify timer resets at checkpoints. Original has more soldiers around the bus. | M | 33825 |
| 86-88 (bus_end to bridge, 3080-3260) | 3080-3260 | Palette darkens to shadowed brown (t86-88). Cliff, cones and pipe art. | bus_end@3050 (unlock). bridge@3300 (cameraMove to 3505,480; bounds 3180-6520, y256-704). | Palette and backdrop. Vertical camera change in ours (y 480) may not match original. Verify. | M | 33824 |
| 88-100 (bridge, 3260-3700) | 3260-3680 | Wooden boardwalk walls with soldiers standing on top (t89-99). Soldiers on platforms. DIE text and death blasts t91-95 (X~3400). BOMB caption t97. Camera stop t90-98. Missile volley t99-101 with smoke trails. | bridge: shielder@3880, bomb@3900, sword@4020, POW@3900 y382, box colliders. bridge_down@3700: mob_boat@4140, fence@4177. No missile before 4700. | No boardwalk or platform soldiers. Original missile volley ~t100 (X~3600) vs our first missile at 4700. Shielders earlier in ours? (orig shielders not seen before t110). | H | 20642 (missiles), 11279, 11302 |
| 100-116 (bridge_down, 3680-4170) | 3680-4170 | Wagon (mob_boat sprite, 35366) rolls in from right at t99-100 (screen-right ~X4160), carries 3-4 soldiers with bazookas and missiles. Bazooka impacts (white balls) t102-113. Wagon wreck t112-113. Camera stops t101-111. | mob_boat@4140 (soldiers on it), shielder@4120, POW@4300 y40. bridge_mid@4037: missile@4700, shielder@4860, bomb@4520/4830, sword@5220 (later). | Wagon position matches (~t99 vs 4140). Missile timing and shield count differ. Wreck state of mob_boat (Mob_Boat_Destroy) present? Verify. | M | 35366, 20642 |
| 116-130 (bridge_mid, 4170+) | 4170 | Camera stops at X~4170 (t116-130). Second wagon from right ~t120 with soldiers. Player shots (white-blue beam) t121, t126-129. | bridge_mid@4037 starts; mob_boat@4634; pow@4890 y40; missile@4700. No camera lock. | Camera stop: ours has no lock here (verify whether player-driven). Second wagon ~t120 vs our 4634 (~t110 entry). | M | 35366 |
| 130+ (out of scope) | | | bridge_end@4600 (soldier_bomb, bzk), river_start@5000. | | | |

## 2. Cross-cutting differences (0-130 s)

| Item | Original | Ours | Pri | Sheet |
|---|---|---|---|---|
| Enemy density (street) | 4-6 on screen, 25-40 total X750-2244 | 6 total in same range | H | 11279 |
| Heavy machine gun crate (H) | ~X2070 at t56-57 | none in street | H | 39079 |
| Destroyed-state size | large persistent rubble piles (t29-68) | single destroyed sprite per shop/house | H | 11302 |
| Missile volley (wagon) | t99-101 from wagon | first missile at X4700 | H | 20642 |
| Blood splashes | t19, t64, t112-114 | not seen in ours captures | M | 32529.gif |
| Bus damage (flash, smoke, wreck) | orange hit-flash, smoke, wreck | verify | M | 33825 |
| Explosions | small/mid/large fireballs with smoke (t3, t9-11, t18-19, t27, t33-41, t54-60, t73-79) | Explosion_* clips exist | M | 11302 |
| Score popups | 100/300/500/1000/5000 | random_score items; popups to verify | L | 39079 |
| Timer | 60 reset at t70 and t81 | verify checkpoint reset | M | 140780 |
| Captions BULLET, BOMB, DIE | t56-57, t84-85, t93-95, t97 | none | L | 39079 |
| GO arrow | none seen at 1 fps | none in ours | L | - |
| Camera stops | t70-81 (bus), t90-98, t101-111, t116-130 | forward policy, lock only for bus | M | - |
| Water, falling objects, collapsing buildings | none seen at 1 fps before t130 | - | L | - |
| POW captives | not confirmed in original 0-130 s (yellow figures t87-95, t119-130 may be players). Blue arrow t2-4 unknown | POW@1325, 1542, 2000, 2350 (y140), 3900 (y382), 4300/4890 (y40) | M (verify) | 11279 |
| Vehicles | green pickups and trucks with riders (t6-11, t14-36, t48-56), wagon (mob_boat) t99-130 | static trucks; mob_boat; jeep only in river_mid (6000) | M | 53213, 35366 |
| Helicopter, plane, boat wreck | none in 0-130 s | aparch only later | L | 20642, 35366 |

## 3. Priority H items

1. Street enemy density and variety: add soldier waves (sword, bomb, bzk, roof climbers) across X 750-2244 to match t13-66 (about 25-40 enemies). Re-time to the original camera X above.
2. Heavy machine gun crate (H) near X~2070 (t56-57).
3. Destroyed-state sprites: the original shows rubble piles (t29-68) across shops X1274-2513. Use a larger rubble prop or stacked debris in the destroyed state.
4. Bridge missile volley: a soldier_missile group near X 3550-3700 (t99-101), not at 4700.
5. Bridge boardwalk soldiers on top of wooden walls (X 3260-3650, t89-99) and death blasts with DIE captions.

## 4. Layering and alignment audit (x 0-3300)

Method: overlay of our prop anchors (Stage0.json) on `Assets/Baked/Textures/Stage0_Main.png` (RGBA, 2x of `33824.png`), checked against original frames at the matching camera X.

- The baked art has transparent holes at X~1274-2600 for the upper facades (y < ~200). Shop props (shop_0..shop_5) must fill them. No double-building was seen on the overlay, which is good. When a shop is destroyed, the hole must be covered by the destroyed sprite, or the sky/black shows through. Verify at t20-35.
- Truck anchors (y=342) sit on the road line of the art (ground ~342-350). OK.
- POW anchors at y=140 sit inside the shop facade region above shop_0..shop_5. They float in front of the shops. Suggest either placing them in a window (y ~ 170-200) on the shop's layer, or confirming they are on the roof-line. Not verified against original pixels.
- Parker (anim_decor@386,341) is set to the background layer. The original blue-clad soldier at X~380 stands in front of the shops. Suggest moving to the play layer or confirming it is only decor.
- Could not do pixel-level sprite alignment without rendering props. Recommend a debug render of the street with props on and off, aligned to the original at t20 (X~990) and t40 (X~1550).

## 5. Needed source sheets (from Assets/Source)

| Sheet | Contents (checked) | Needed for |
|---|---|---|
| 11279.png | Soldier and POW strips (578x2980) | street soldiers, POW |
| 11302.png | Explosion strips (592x3357) | blasts t9-80 |
| 39079.png | Items and icons (819x1917) | H crate, score popups, captions |
| 32529.gif | Blood | t19, t64, t112 |
| 53213.png | Green vehicle strips (702x3380) | street pickups and trucks |
| 35366.png | Mob boat (wagon) and its wreck, plane | bridge wagon t99-130 |
| 20642.png | Projectiles (bombs, missiles, bazooka rounds), helicopter and weapons | missile volley t99-101 |
| 33825.png | Bus clips (Bus_Damaged, Bus_Smoke, Bus_Wreck) | bus fight t70-86 |
| 33824.png | Background master | backdrop palette t86-88 |
| 51797.png | Game over | not in 0-130 s |
| 140780.png | HUD (timer, arms, bomb) | timer reset |

## 6. Open questions (verify at 4 fps)

- Blue down-arrow marker t2-4: POW marker, item drop, or parachute indicator?
- BULLET, BOMB and DIE captions: are these in-game text or enemy shouts?
- POW captives: confirm whether any POW appears in 0-130 s.
- Camera stops t90-98, t101-111, t116-130: player-driven (forward policy) or scripted lock?
- Timer reset at t70 and t81: checkpoint behavior.
