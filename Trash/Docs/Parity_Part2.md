# Parity Part 2: original t = 130..251 s vs Stage0 (x 4000..12540)

Source: `Assets/Source/videoplayback (2).mp4` (480x360, 29.5 fps). Sampled at 1 s (6x4 contact sheets at 240x180, then 3x2 sheets at native 480x360 for detail). Times are +-1 s. Sprite identities are by eye, not by sheet cut. Scratch frames: `scratchpad/par2`.

## 0. Time to x mapping (visual landmarks, not camera data)

| Orig t (s) | Approx Stage0 x | Landmark |
|---|---|---|
| 130-137 | 4000-4700 | bus wreck (bus boss died ~132), bridge_mid/bridge_end |
| 137-170 | 4700-6220 | river_start / river_mid (tower, jeep, heli) |
| 170-177 | 6220-6500 | river_end, sand village entry, Slug boarding |
| 177-226 | 6500-8980 | downtown (rooftops) |
| 226-250 | 8980-12350 | openland_1..3 (rocket launchers, ladder tower) |
| 250-251 | ~12350-12540 | openland_4, walker enters |

Stage0 section triggers are in `Assets/Stage/Stage0.json`.

## 1. Segment table

Pri: H = visible gap in the parity footage, M = mismatch in count/timing/FX, L = polish.

| Orig t | Original has | Ours has (Stage0) | Delta | Pri | Sheet |
|---|---|---|---|---|---|
| 130-137 bus wreck | Wooden bus/cart with barrels shot 130-131; black smoke + 3 blasts 131-133; wreck heap with barrels stays until ~138 | bus boss mb_truck x2995 (before scope); bridge 3300-4600 | Verify our bus death blast and wreck heap persist (we may remove the wreck); timing ok | M | 33825 (bus Destroyed/Broken debris) |
| 136-139 infantry + HMG | 2-3 rebel infantry (sword, grenade); HUD ARMS inf -> 199 at 137 (HMG pickup) | bridge_end: soldier_bomb 5060, soldier_bzk 5220, bomb wave; POW RL 4300, SG 4890 (bridge_mid) | Our first gun pickup is RL/SG; orig gives HMG ~200 rounds at ~137 | M | 11279, 39079 |
| 139-150 red-X tank | Red-X round-turret armoured tank rolls in from right (139) with 3-4 riflemen; white water splash under wheels 140-141 and 145-150; orange hit flash 143-146; large fireball + smoke at 147; blood burst 150 | river_start 5000-5600: soldier_bomb 5500/5640/5770, missile 5840, sword 5760, 2x soldier_hang 5735/5865, flying_bzk wave. No armoured tank before 6000. Water = anim_decor Stage0_Water 5639 only | MISSING red-X armoured tank (ours has the same sprite family only as sarubia @12540). Hang soldiers not seen in orig 139-170 (verify). Tank-wheel water splash FX missing | H | 53213 (Hit/Destroyed), 11302, 32529.gif |
| 148-153 cart + bazooka pair | Rebel bazooka + rifle pair 148-153; fireball explosions 148-150; loaded wooden cart (barrels + cloth) rolls in from right 152-153 | soldier_missile 5840, soldier_bomb 5640; no cart in 5600-6200 (guard_post is bg-only at 5800) | MISSING loaded cart (Props truck, loaded variant, exists in Props.json but unused in Stage0) | M | baked Truck_* (Props) |
| 154-170 siege tower + jeep + heli | Wooden siege tower on wheels with 3-4 rebels on top (154-169); green jeep with gunner enters from left 157, fires rockets 163-164, retreats right 165; attack heli enters top-right 155, drops 2 bombs 157-158, stays in top band (no vertical chase), bursts and falls 160-161 | guard_post 5800: background decor, hp 0, not destructible, no crew; aparch_river gated at 5600+ (dy 125); jeep 6000 (groundSnap, 4 rockets) | MISSING destructible siege tower with crew (H). Jeep side/entry differs (orig enters from left) (M). Heli: orig = bombs + burst death; verify ours drops bombs, not only missiles (M) | H | 20642 (heli bombs/destroy), baked guard_post |
| 169-171 tower death | Tower destroyed 169-170: fireball 170, dark smoke column; timer reset 46 -> 60 at 169 then counts down (60 -> 49 by 251) | river_end 6220: backdrop third, plane decor 6470, soldier_bomb 6650, soldier_sword 6900, flying_bzk wave. Our HUD shows 60 constant in captures | Tower collapse FX (H with row above). Timer behaviour not implemented (L) | M | 11302 (fire), 29720 (digits) |
| 172-174 vehicle boarding | 'IN' arrow (down arrow + IN tag) 172-173 above grey Metal Slug on a plank; player boards 173-174; HUD ARMS -> CANNON inf 10; top-left orange bar becomes vehicle HP | Nothing. No player-mountable vehicle, no IN prompt in Stage0 or docs | MISSING Slug vehicle mode for the whole last course (H) | H | new Slug sprite (not in listed sheets; probe 32522, 33824); 29720 (IN glyph) |
| 174-177 flyers | 2 grey-blue bomber/plane sprites cross 174-177, blue tracer streaks 175-177 | anim_decor Plane 6470 (farBackground), 1 plane, no fire | 1 vs 2+; no tracer fire (M) | M | 35366 (plane 1-2) |
| 177-200 rooftop melee | 8-12 rebels visible on roofs; sword rebels leap roof to roof (181, 196-201); bomb throwers with black ring blasts (188, 203); tall fire pillars 181-183, 191-193, 194; green "50" tank (MissleTank family) rolls between houses 191-195 and 205-207; Slug cannon refill +10 at 186 (7 -> 17); score popups red 100 (188) | downtown_start 6500: POW 7200 random_score, 7430 bomb_supple, 7865 random_score; soldier_bomb x2 (7189, 7396), missile 7599, bomb 6980, bzk 7300; flying_bzk wave. downtown_mid 7400: POW 8190 bullet_supple, 8270 random_score, 8900 gunitem_mg; ~7 soldiers; wave | MISSING green "50" tank (H). MISSING cannon refills (see Slug row). Tall fire-pillar explosion absent (M). Ours has 6 POW/npcs in 6500-8900 but orig shows no rescue/weapon swap in this span (H for pickups) | H | 53213, 11302, 32529.gif, 29720 |
| 200-226 rooftops burn | Rebels keep dropping onto roofs (202-219); house collapse with fire + black plumes 204-210; burning columns 222-225; cannon refill +10 at 208 (12 -> 22); score popups 300/1000/1200 at 208-214 | downtown_end 8700: soldier_bomb 8878, missile 9153, bomb 9200, sword 9340; flying_bzk wave. openland_start 8980: POW MG 9250, bomb 9420, sword 9660; flying_bzk wave | Second green tank missing (H). Fire columns / plumes (M). Cannon refill +10 at 208 has no counterpart (H, Slug) | H | 11302 (fire column, Enemy_Bomb_Exp), 53213 |
| 226-237 open land | Planks and wood debris fly (226-228, 233); green tank 226-229; orange loaded cart burns 229-231; tank blasts 233-236; smoke columns 236-237; HUD pistol blip 236-237 (Slug ejects, CANNON 16 kept) and CANNON HUD again at 238 | openland_1 9450: POW RL 9950; bridge_block 10098, big_house 10470 (damaged at 12 hp), tower 10808; soldier_bomb 10440/10560 (y1195), missile 10813, missle_tank 10900 (groundSnap); flying_bzk wave 5 s | Slug eject/re-board (H, part of Slug). Loaded cart (M). Wood-plank debris rain (M) | M | 39079 (cannon ammo), 16340 (wood debris) |
| 238-243 rocket launchers | 2-3 twin-cylinder rocket-launcher vehicles (MissleTank family) on rooftops; one fires a rocket up at 242; green military truck with soldiers rolls in 243 | openland_1 has 1 missle_tank (10900). openland_2 10350: bridge_block 11122, big_house 11494, tower 11832, POW bomb 10696, soldier_missile 11837 | MISSING second and third launcher (H). Green truck with soldiers missing (M) | H | 53213 (MissleTank Shooting/Driving) |
| 244-249 ladder tower | Rebels climb a wooden ladder-tower and jump off (244-246); tower bursts into fire column 247-249 (rebels blown up); red-X armoured tank again 247-249 with hit flashes; orange loaded cart 248 | openland_2/3: tower 11832, 12856; openland_3 10 soldiers + btank 11900 (fixed, 120 hp) + POW SG 11265, FS 11975; aparch in openland_3 wave (delay 9 s) | Orig has no heli in 226-250 (verify) and no fixed-turret tank: our btank may be the red-X tank (confirm sprite) (M). Tower burst + ladder crew (M) | M | 53213, 20642 (only if heli appears), 11302 |
| 250-251 walker enters | Tower bursts again 250 (fire column + smoke). Grey armoured walker (cannon arm, smoke, blue shots) appears 251 at right edge; scope ends before gate ~257 | openland_4 12350: sarubia (red-X walker, gate lock) at 12540, soldier_bzk 12950, sword 13030, bombs 13260/13310; boss trigger 12900 | Sarubia spawn x ok within ~200 px. Orig walker look not identified (verify against our sarubia sprite) (M) | M | 40204 (boss family ref), 33824 |

## 2. Weapon pickups in the last course (orig t 172-250, ours x 6220-12350)

- HUD evidence: weapon stays CANNON (Slug) from 174 to 250, except a pistol blip at 236-237. Only 2 pickups are visible: cannon refills +10 at ~186 and ~208. No POW rescue or weapon swap is visible.
- Pre-scope HMG pickup at ~137 (ARMS inf -> 199), HMG runs out ~169.
- Ours in x 6500-12350: 11 POW/NPC drops (7200 random_score, 7430 bomb, 7865 random_score, 8190 bullet, 8270 random_score, 8900 MG, 9250 MG, 9950 RL, 10696 bomb, 11265 SG, 11975 FS). The orig has 2 cannon refills here.
- Crates vs POW: no crate sprite and no rescued-rebel frame seen at this resolution. Cannot confirm either way. Zoom needed if a rule depends on it.
- Recommendation: if the Slug mode is built, replace most of the 11 POWs in 6500-9000 with 2 cannon refills (about x 6900 and 7500, matching 186 and 208 timing) and keep 2-3 POWs in openland for pistol-era parity. Otherwise cut the POW density to about 1 per 1500 px.

## 3. Cross-cutting findings

- Helicopter: no vertical chase in orig. It hovers in the top band, strafes horizontally, drops 2 bombs, then bursts and falls (160-161). Keep this behaviour.
- Explosion variants seen: small/mid orange burst; large fireball with black smoke; tall fire pillar (183, 191-193, 222-225, 235, 250); black ring blast from enemy bomb (131, 188, 203, 220); white flash (170, 240); red blood splash (136, 150, 196-199, 212).
- Debris: wood planks flying from roof collapses (226-233); rubble heap left after tower and bus deaths (138, 170).
- Water: no river visible in orig 130-170, only ground splash under tank wheels (140-141, 145-150). Our Stage0_Water (5639) may be too prominent.
- Falling objects: soldier bombs and rebels falling off roofs (181, 196-201); planks; barrels from bus.
- Collapsing structures: bus (132), tower (169, 250), houses (181-194, 204-210), ladder tower (247-249).
- Score popups: red values 100, 300, 1000, 1200 at 188 and 208-214.
- GO arrow / camera stops: only the IN arrow at 172-173. No camera stop observed in 130-250.
- Shielders and parachute soldiers: not clearly identified in 130-250 (check 136-139 region; orig has no obvious shield sprite).
- Hanging soldiers: not observed in orig 130-250; ours has 2 at 5735/5865. Verify before keeping.
- HUD: top-left ARMS/BOMB, cannon count, timer (60 reset at 169, counts down to 49 at 251). Our HUD timer is constant 60 in captures.

## 4. Priority list (H)

1. Slug vehicle mode (boarding at ~172, IN prompt, CANNON ammo, ejection and re-board at 236-238, vehicle HP bar), replacing the pistol/POW flow from x 6220.
2. Red-X armoured tank at ~140-150 (river_start) and ~247-249 (openland). Confirm sprite family.
3. Green "50" tank (MissleTank family, 53213) at ~191-207 and ~220-226, plus 2-3 rocket launchers at 238-249 (ours: 1).
4. Destructible siege tower with crew at ~154-170 (ours: background decor).
5. Remove or thin POW density in downtown (6500-8900) to match orig's no-pickup span.
6. Helicopter bombs at 157-158 and burst death at 160-161 (verify).

## 5. Asset sheet needs (Assets/Source)

- 11279 POW / helpful NPCs: POW variants for the HMG rescue at 137 and any rescued rebel.
- 20642 helicopter: bomb drop and destroy frames (154-161).
- 35366 boat + plane: plane 1-2 (174-177).
- 11302 explosions: fire pillar (Explosion_Huge?), enemy bomb ring, smoke columns.
- 39079 items: cannon ammo refill icon (186, 208), HMG capsule (137).
- 32529.gif blood: red splatter (136, 150, 196-212). Not cut yet.
- 51797 game over: not needed in this range.
- 53213 green tank: "50" tank Hit/Destroyed/Driving, plus MissleTank Shooting for launchers.
- 33824 background master: siege tower, loaded cart, roof assets; verify.
- Not in any listed sheet: Slug (grey Metal Slug, IN/CANNON) and walker at 251. Probe 32522 and 33824 first.
- 29720 font: IN glyph, timer digits, score popups.

## 6. Limits

- Sampling is 1 s, and sprite identities are inferred from 480x360 frames. Small enemies (shielders, hang soldiers) can be missed.
- Time-to-x mapping is visual. Scroll speed varies (about 46 px/s in 137-170 vs about 140 px/s in 226-250), so per-row x values are approximate.
- Weapon crate vs POW cannot be settled from these frames.
