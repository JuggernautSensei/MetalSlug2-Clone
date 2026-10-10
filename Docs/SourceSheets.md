# Raw Source Sheet Catalog (Assets/Source)

Scope: every raw sprite sheet in `Assets/Source/` (spriters-resource / ripper rips, native 1x pixel art; the game renders at 2x).
Compared against `Assets/Source/sheets.json` (pilot clips) and `Assets/Baked/anim.json` (780 clips) and `sprites.json` (228 sprites).

Coordinates are sheet pixels, top-left origin, 1x. Frame counts are estimates (projection and connected-component scans where the
sheet is grid-like; visual counting otherwise). Credit/label areas are listed under "Exclude" and must not be fed to `auto`.

## 0. Method and conventions

- Key colours were measured from corner pixels and top colour histograms. Backgrounds are not uniform across sheets; some sheets
  use two background colours (e.g. green + magenta gutters). Alpha-keyed sheets have RGBA 0 backgrounds.
- Sheets 33824, 143506, 53213, 33825, 32522 and 20587 are "art sheets" (large backgrounds, bus/tank/effect compilations). Only some
  of their content maps to our clips.
- Baked clip names are quoted exactly as they appear in `anim.json`.
- Mission 4 credit: the 33824 sheet is "MISSION 4_ OPERATION HONG KONG" (ripper C2000MC/Futaokuu). Stage0_Main is its art.

## 1. Summary table

| Sheet | Size / mode | Background key | Content (short) | Maps to baked / game | New vs baked | Verdict |
|---|---|---|---|---|---|---|
| 143506.png | 1506x1860 RGBA | none (2 opaque blues) | Metal Slug 2X title logos, logo variants, badges | TitleBackground (640x360) | Logo art only | Skip |
| 29720.png | 566x1357 RGB | white / green (0,248,0) / pink (255,204,204) | Mission-start font (A-Z 0-9), CONTINUE digits, GO! signs, JUMP, IN/OUT, P1/P2 icons, points font, weapon-storage (MS6/7, exclude) | UI_Mission_* (37 sprites, done) | GO! signs, CONTINUE digits, IN/OUT, JUMP | Mid |
| 32522.png | 1983x19513 RGBA | (153,217,234) | Large mixed compilation: small olive/brown creatures or items, blue water drops, fire/smoke | none | unidentified | Probe later |
| 33825.png | 567x2199 RGB | white + (0,251,0) edit markers | Double-decker bus: Idle, Opening, Damaged, Window break, Broken debris, Destroyed + green-tank/bus-garage art | MainTruck_* (10 clips) | Opening, Damaged, Window break, Destroyed | Cut (mid-high) |
| 38098.png | 288x472 RGBA | alpha | Delivery boy (Order_*) | Order_* (7 baked) | none (pilot done) | Done |
| 40204.png | 4624x3317 RGBA | (153,217,234) + (0,255,0) | Big Shiee boss (124 clips) | BigShiee_* (124) | none | Done |
| 46546.png | 318x516 RGB | magenta (255,0,255) | Parker NPC | Parker, Parker_Run, Parker_Run_2, Parker_Stopping | none (pilot done) | Done |
| 53213.png | 702x3380 RGB | (153,217,234) | Green tank: Idle, Driving, Accelerating, Shooting, Destroyed, Hill angles, Appearing from hill, Hit | MissleTank_* family (36) | Driving, Accelerating, Hill, Appear, Hit | Cut (mid-high) |
| 11279.png | 578x2980 RGB | white (255,255,255) + green (0,128,0) credit box | P.O.W./Helpful NPCs: gold POW (Hyakutaro), Rumi/Madoka, Scholar/Doctor, CEO President, tan-suit POW, cyan UNUSED row | Older_* (11, POW) | Bound/cheer/hit/rescue variants | Mid |
| 16340.png | 328x1550 RGB | green (0,248,0) + magenta (248,0,248) | Debris: wood planks/sticks, stone chunks, metal balls, splinters | Wood1-6, Metal1-12, Rock1-3 | More variants | Low |
| 20587.png | 605x6950 RGB | green (0,248,0) | Weapon/bomb SFX compilation (HM, rocket, flame, shotgun, laser, grenades, capsules) | HM_Bullet_*, ShotGun_Bullet_*, Bullet_Destroy_Effect | Rocket, flame, laser, misc | Low (out of spec) |
| 51797.png | 617x243 RGB | magenta (255,0,255) gutter | GAME OVER screen (two panels) | none | Yes (no game-over sprite baked) | Cut now |
| 140780.png | 97x131 RGBA | alpha | HUD: ARMS, BOMB, CANNON, TIME LEVEL bars, icons | PlayerStateUI_Bomb/Cannon, TimeNumberUI, PlayerHPUI | Duplicates | Skip |
| 33824.png | 7104x1234 RGB | white (255,255,255); magenta (255,0,255) as sprite key in some parts | Mission 4 background master (3 palette variants), mini UFO, buildings, slope, bus parts | Stage0_Main (2x of this art), Stage0_Background_0/1/2 | Correct layers | Cut now (layers) |
| 11302.png | 592x3357 RGB | green (0,248,0) | Explosions: normal S/M/B, grenade S/B, mine, fire bomb (air/ground), fire effect, big on water, huge | Explosion_Small/Mid/Large/Huge, Mini_Explosion, Enemy_Bomb_Exp, PlayerBombExplosion | Fire bomb, water, grenade-big, mine | Cut (mid-high) |
| 39079.png | 819x1917 RGBA | alpha | Item icons: food, medicine, animals, weapon capsules, ammo HUD icons | Item_* (14 clips) | Most items | Low-mid |
| 35366.png | 320x280 RGBA | alpha | Mission 4 objects: boat intact + wrecked, plane (2) | Mob_Boat_Idle/Destroy, Plane (2) | Duplicates | Skip (verify) |
| 20642.png | 776x765 P (transparent) | (0,248,0) alpha 0 | Helicopter (aparch): Flying, Turning, Missile launcher, Chopper blades, Missile+Bomb flights, Effects | Aparch_* (33), Aparch_Turn_L/R sprites | Missile flight, flying, launcher | Cut (mid-high) |
| 32529.gif | 504x407 P (transparent) | alpha | Red blood splatter effects (static sheet) | none | Yes (no blood clips baked) | Cut now |

Counts below use the full-file pass. "Done" = already in `sheets.json` (pilot) or baked as a legacy clip.

## 2. Per-sheet detail

### 2.1 143506.png (Title / logo art)
- Size 1506x1860 RGBA, opaque. No key. Top colours (153,217,234) and (112,146,190) are the sky/panel backgrounds, not keys.
- Content: big METAL SLUG 2X logo (top, approx y 0-560), logo variant row (approx y 610-1050), second logo row with
  "SUPERVEHICLE" badges (approx y 1076-1320), bottom panels (dark-blue and red gradients, logo badge, approx y 1340-1858).
- Exclude: "LETTERS / RING SIZES" caption text (top) and "SUPERVEHICLE" labels.
- Baked: TitleBackground (640x360) exists. Use only if a title screen needs the logo as a sprite; not a sheet-cut target.

### 2.2 29720.png (HUD fonts / signs)
- Size 566x1357 RGB. Keys: white (255,255,255), green (0,248,0) edit sprites, pink (255,204,204) original sprites.
- Done: UI_Mission_A-Z, 0-9, Excl, Question (37 clips, rows y 0-100 in the pink/green band).
- Sections (approx):
  - "CONTINUE? SCREEN NUMBERS": 0-9 large chrome digits, y 100-160 (about 11 glyphs). Not baked.
  - "GO! SIGNS - NORMAL": grid, y 160-360, about 6 rows x 14 glyphs (green-background cells, 45 px tall each).
  - "GO! SIGNS - DIAGONAL": y 360-550, about 4-6 rows x 14 glyphs.
  - "GO! SIGNS - UP 'N' DOWN": y 550-620, 2 rows x 14.
  - "JUMP! SIGN (MS5 exclusive)": y 630-690, 9 glyphs + text.
  - "MELEE ATTACKS ONLY! (MS5 exclusive)": y 700-760, 8 X-icons (unused).
  - "PLAYER P1 / P2 ICONS": y 770-880.
  - "IN / OUT SIGNS": y 880-960 (IN 9, OUT 9 arrows).
  - "POINTS FONT" glyph strip: y 960-980 (A-Z uppercase + digits + symbols).
  - "WEAPON STORAGE SYSTEM (MS6/MS7 exclusive)": y 980-1060, exclude.
  - "MISSION START!" banner + "CREDITS TIME" art, y 1060-1357: exclude (text and banner).
- Exclude: credits text block (y 1180-1240), "MISSION START!" caption, weapon-storage block.
- Recommendation: GO! signs (normal + diagonal + up/down) and CONTINUE digits are the only gaps that look needed by the HUD.

### 2.3 32522.png (unidentified mixed sheet)
- Size 1983x19513 RGBA. Key (153,217,234) (also the alpha-0 background in the sheet's bottom-right corner).
- Layout: about 60+ labelled rows of small sprites (olive/brown bags or creatures, grey sprites, blue water droplets, flames,
  dark smoke, bomb-like shapes), labels are too small to read at 1x; needs a 3x crop of individual labels.
- No mapping to baked clips identified yet. Treat as a probe target only. Do not cut from it until labelled.

### 2.4 33825.png (double-decker bus)
- Size 567x2199 RGB. Keys: white (255,255,255) background; green (0,251,0) = edit markers (must be excluded); pale tan labels.
- Sections (approx, sheet y):
  - "IDLE": 5 frames (x 0-420 across the first row, y 0-100) plus 2 wall fragments.
  - "OPENING": about 11 frames (door open sequence, y 100-260).
  - Bus body variants: 11-14 rows of damaged/burning bus frames, y 260-470.
  - "WINDOW BREAK": 4 rows x 8 cells (green-marker cells; exclude marker colour).
  - "BROKEN OFF DEBRIS": about 20 pieces, y 1025-1150.
  - "DESTROYED": 2 wreck frames, y 1160-1250.
- Exclude: bottom credit text (y 1560-1640), the photo strip at y 1640-1700, all green marker cells (0,251,0).
- Baked: MainTruck_Begin (25), MainTruck_Idle_0..3, MainTruck_Ladder_0..3, MainTruck_Die, MainTruck_Die_Front, MainTruck_Bump, MainTruck_Sideparts, MainTruck_Deco_Destroy, MainTruck_Big_Win_Destroy_*.
  Sheet "Opening", "Damaged" and "Window break" states are not in the baked set. Mid-high cut target.

### 2.5 38098.png (Delivery Boy NPC) — DONE (pilot)
- Size 288x472 RGBA. Key: alpha. Clips in sheets.json: Order_Run_Right (mirror), Order_Stopping, Order_Idle, Order_Recoil, Order_Turn, Order_Run_2.
- Baked (legacy) Order_Run_Left/Right (8), Order_Idle (9), Order_Recoil (4), Order_Run_2 (6), Order_Stopping (3), Order_Turn (2) already exist.
  The pilot entries are duplicates (skipped unless replace:true).

### 2.6 40204.png (Big Shiee boss) — DONE
- Size 4624x3317 RGBA. Keys: (153,217,234) and (0,255,0). 124 clips in sheets.json (`BigShiee_*`), details in `Docs/BigShieeSheet.md`.

### 2.7 46546.png (Parker NPC) — DONE (pilot)
- Size 318x516 RGB. Key: magenta (255,0,255). Exclude row y 484-516 (label). Clips: Parker_Run (row 0), Parker_Stopping (row 1), Parker_Run_2 (row 9).
- Baked: Parker (7), Parker_Run (12), Parker_Run_2 (12), Parker_Stopping (11). Pilot duplicates; no new value.

### 2.8 53213.png (green tank)
- Size 702x3380 RGB. Keys: (153,217,234) sky-blue background; no marker colours.
- Sections (approx sheet y): "NORMAL ANGLE IDLE" (2 frames, y 0-30), "DRIVING" (about 11 frames, y 40-70), "ACCELERATING FORWARD" (about 11), "ACCELERATING BACKWARD" (about 11), "SHOOTING" (4-5 per row, 2 rows), "UNUSED" (2 rows), "DESTROYED" (about 12 frames, y 210-240), "HILL ANGLE: IDLE", "DRIVING" (uphill/downhill, about 11 each), "APPEARING FROM HILL" (about 9), "HIT" (about 8 frames per row), "UNUSED" marked rows.
- Frame count estimates: idle 2, driving 11, accelerating 11+11, shooting 9+9, destroyed 12, uphill/downhill 11+11, appearing 9, hit 8.
- Exclude: all section label text ("NORMAL ANGLE", "DRIVING", etc.), the "UNUSED" tags, the blue background.
- Mapping: BTank (fixed turret tank) does NOT match this sheet (BTank: Idle, Animing_Up, Fire, Destory, Proj). EnemySpecs.md Section 2.3 defines BTank as a fixed turret.
  The closest baked family is MissleTank (36 clips with Up/Down, Move, Turn, Shot, Proj, Destroy). Verify which enemy this green tank is before cutting.
- Cut target: driving/accelerating/hill/appearing/hit states. None of these appear in baked MissleTank or BTank clip names.

### 2.9 11279.png (POW / Helpful NPC art)
- Size 578x2980 RGB. Keys: white (255,255,255) background; green (0,128,0) = CREDITS box and the (0,255,0)-style green boxes behind effect frames; cyan (0,255,255) row = UNUSED.
- The source is not rebel soldiers. It contains P.O.W.s and helpful NPCs. Sections (approx sheet y):
  - "Normal P.O.W.s / Hyakutaro Ichimonji" header: y 0-15 (exclude).
    - Gold POW cheering and bound POW rows, about 12 rows x 8-12 frames, y 18-530. Includes straw/dust particle frames (x 120-430, y 58-240).
    - "The next rows are Hyakutaro-only frames" label: y 525-535 (exclude).
    - Hyakutaro-only frames, about 6 frames each for blue wind / sword effects, y 539-670.
    - Gold POW jump / slash row, about 10 frames, y 672-719.
  - "Rumi and Madoka Aikawa" header: y 721-729 (exclude).
    - Scout girl with backpack, about 8 rows x 10-12 frames, y 733-1026.
  - Scholar/Doctor section, about 7 rows x 10-14 frames, y 1033-1540 (includes an "alternate colors for the suitcase" row and a palette legend, top right: exclude legend swatches).
  - "CEO President" header: y 1540-1550 (exclude).
    - Bald CEO in grey suit with briefcase: walk, run, shoot, hit and vanish, about 10 rows x 12-16 frames, y 1550-2180.
    - Green-box vanish/death effect frames, about 2 cells each, y 2089-2177 and y 2328-2415 (green box is an effect background; key it out).
  - Tan-suit POW (helpful NPC): raise-arm cheer row (about 16 frames, y 2180-2228), hide/cower row (about 16, y 2231-2325), green-box effect cells (y 2328-2415).
  - Cyan background row, UNUSED, about 15 run frames, y 2419-2460.
- Exclude: CREDITS block (y 2466-2980, the green box with "Brought to you by Gussprint"), all section labels, "(UNUSED)" tag at x 460-520, y 2440-2448.
- Mapping: Older_* (11 baked clips: Idle 8, Move 12, MoveDown 13, Jump 7, Escape 4, Item 29) is the POW old-man NPC (EnemySpecs.md 4.3).
  It is not obvious which sheet section Older uses. The gold Hyakutaro section (bound/cheer/hit) is a candidate for POW variants. Verify visually against Older_Idle before cutting.
- Other baked clips (Hostage, Soldier, Alian) are not in this sheet.

### 2.10 16340.png (Debris sprites)
- Size 328x1550 RGB. Keys: green (0,248,0) cell backgrounds; magenta (248,0,248) gutter/row separators.
- Sections (approx, original y): top half (y 0-775) = left column: wood planks (2 rows x 10), wood sticks (2 rows x 5), sticks/splinters (2 rows x 5),
  4 rows of wood fragments (5 each), small wood bits (10), splinter piles (2 rows x 5), metal balls (1 row x 8), metal ball clusters (2 rows x 4),
  single metal balls (2 rows x 8), pebble groups (2 rows x 4), metal chips (1 row x 9), rock chunks (1 row x 9).
  Bottom half (y 775-1550) = right column: stone chunk clusters (4 rows x 4), stone pebble clusters (4 rows x 4), wood planks (2 rows x 4), wood sticks (2 rows x 4 and 8), splinter clusters (4 rows x 4), wood chips (2 rows x 4), chip/particle strips (1 row x 8).
- Exclude: credits text block (sheet x 0-330, y 1488-1550, "Debris sprites (Metal Slug 5 via Sprite Viewer)...").
- Baked: Wood1-6 (10 frames each), Metal1-12 (2-10 frames), Rock1-3 (8 frames each). Sheet adds more variants but these are particle debris; low priority.

### 2.11 20587.png (weapon and bomb SFX compilation)
- Size 605x6950 RGB. Key: green (0,248,0). Magenta (255,0,255) and dark-red (152,0,0) used as sprite colours inside some rows, not as keys.
- Sections (approx sheet y): heavy machine gun (rows at y 0-60, 6x8 grid of bullets), rocket launcher normal (y 210-300), rocket "Big" (y 300-340), rocket trail/smoke (y 370-430),
  flame shot original and deluxe (y 470-1100, 20+ rows of flame frames), laser gun (y 3080-3200), X-ray laser (y 3180-3260), enemy chaser SFX (y 3320-3400),
  iron lizard (grenade) (y 2290-2400 and other grenade rows), drop shot (y 2400-2440), super grenade (y 2420-2470), zantetsu sword (y 2490-2580),
  shotgun unused (y 2640-2800), shotgun nightmare (y 2880-3080), dual machine gun (y 3620-3720), thunder cloud (y 230-340 in column 3), mobile satellite (y 460-600, column 3),
  bomb SFX stick grenade (y 900-1050), fire bomb normal/big (y 1050-1120), underwater bomb (y 1120-1170), armor-piercer shell (y 1220-1260),
  weapon capsules (letters H R F L S G I D Z T etc., rows y 3420-3500), ammo capsules (y 3500-3630), example panel (y 3620-6200, exclude).
- Exclude: example comic panel (y 3620-6200), credits box (y 6740-6950), section labels.
- Relevance: HM_Bullet_*, ShotGun_Bullet_* and Bullet_Destroy_Effect already exist. FS / RL / EC items are out of the current game spec (EnemySpecs/PropSpecs).
  Low priority.

### 2.12 51797.png (GAME OVER screen)
- Size 617x243 RGB. Key: magenta (255,0,255) at the image corners (alpha 0 on 8.5% of pixels).
- Contents: two side-by-side panels (x 0-308 and x 309-617) of the same GAME OVER text in sand, helmet on the first panel (top right, approx x 230-310, y 50-130). The second panel has no helmet.
- Exclude: nothing. The art is a single background; one panel is enough.
- No game-over sprite is baked. Cut now. Recommended: use the helmetless panel (or crop the left half and key the magenta corners) as a full-screen sprite at 2x.

### 2.13 140780.png (HUD pieces)
- Size 97x131 RGBA. Key: alpha (78.5% transparent).
- Contents: ARMS panel (top left), BOMB and CANNON tabs (top right), a horizontal ammo-bar strip (y 20-30), "TIME LEVEL" label and bar (y 45-70), grey grenade icon with "X" (y 85-95), colour bars (y 100-140) and "Unused" tag.
- Exclude: credit text "Division ... 6th-divisions-den.com" (y 112-131) and "Unused" tag (y 67-75).
- Baked: PlayerStateUI_Bomb, PlayerStateUI_Cannon, TimeNumberUI, PlayerHPUI exist. Treat as duplicates. Skip.

### 2.14 33824.png (Mission 4 background master) — special analysis
- Size 7104x1234 RGB. Background: white (255,255,255) (113,395 px). Magenta (255,0,255) appears as a sprite key in the bus/weapon-block area, not as a global key.
- Credit (exclude): "MISSION 4_ OPERATION HONG KONG ... NO CREDIT NEEDED" at approx x 6036-6810, y 675-712.
- Contents (approx sheet x ranges, from 5 vertical slices of 1421 px):
  - x 0-1420: orange sepia city. Market street with shop signs (青菜水果, 燒臘, 飯店, 司公易館), tile street, three palette variants of the same layout (orange, green-grey, dark grey). Shop signs and street colour strips ("STREET COLORS", "WATER COLORS") at y 240-270, 15 shop-sign pieces at y 230-330. Mini UFO sprites and "ADDED BONUS: MINI UFO" block (y 1000-1190).
  - x 1421-2840: second city street (orange and grey variants), rock-pile slope (x 2130-2840, y 300-640), wooden fence segments (x 1968-2560), bullets/grenade icons, street light frames (x 1800-2140, y 370-510), cone and bin props.
  - x 2842-4260: dark-green/orange building columns (x 2840-3120 green tile pattern), tree (x 3140-3420, y 170-340), slope roofs (x 3120-4260, y 0-500), wall tile blocks (x 3290-3320, y 480-880).
  - x 4263-5684: broken hut row (x 4263-4960, y 560-900), houses along slope in three variants, skyline of towers (x 4960-5684, y 0-540).
  - x 5685-7104: Mission 4 header and credit (exclude), hut rows (sepia, grey, green), sunset sky backdrops (x 6100-6560, y 100-360, purple/orange; x 6560-7104 pale yellow), bus (x 6990-7110, y 720-870 magenta-keyed), tank and boat (x 6080-6360, y 740-810).
- Layer correspondence to Stage0_Main (verified numerically):
  - Stage0_Main is the 2x upscale of this sheet (1x = Stage0 full-res / 2).
  - Exact 24x24 pixel matching: 237 of 400 random opaque patches from the 2x-downsampled Stage0_Main found exact matches in 33824 (0 of 60 at native scale). Remaining patches were in pieces with alpha edges or colour variants.
  - Each match gives a translation T = (Stage0_1x - sheet_1x). Clusters:
    - T=(-415,-531): 85 hits. Sheet x 451-2870, y 531-855 -> Stage0_1x x 36-2455, y 0-324 (orange street and slope).
    - T=(-536,-684): 53 hits. Sheet x 3029-3791, y 828-1013 -> Stage0_1x x 2493-3255, y 144-329 (second house row).
    - T=(-565,333): 24 hits. Sheet x 3809-5069, y 19-266 -> Stage0_1x x 3244-4504, y 352-599 (slope houses, lower).
    - T=(75,493): 13 hits. Sheet x 4085-4356, y 55-119 -> Stage0_1x x 4160-4431, y 548-612 (small hut piece).
    - T=(-95,-531): 10; T=(-159,-531): 6; T=(-191,-531), T=(-223,-531), T=(33,-531): 3-5 each (small pieces on the same row as cluster 1).
  - Relative placement: cluster 1 vs cluster 2 differ by (121,153) 1x px (242,306 in Stage0 full-res); cluster 3 is offset by (-150,+864) relative to cluster 1.
    These are the layer-to-layer offsets Stage0_Main was built with. If building layers in the game look misaligned, the offsets in these clusters
    are the authoritative placement for re-cutting. Verify in-game before changing anything; the match only proves where the art came from, not what the game expects.
- Recommendation: for the layer fix, re-cut the Stage0 building layers from this sheet at 1x, applying scale 2 in sheets.json (per AssetFormat.md), and place each piece at its cluster T.

### 2.15 11302.png (explosion compilation)
- Size 592x3357 RGB. Key: green (0,248,0). Dark green (0,128,0) appears in label boxes and the credit box.
- Sections (approx sheet y, 1x):
  - NORMAL EXPLOSION (SMALL): 3 rows of 11-12 frames, y 0-190 (plus two black/white circle frames at x 0-100).
  - NORMAL EXPLOSION (MEDIUM): 3 rows of 7 frames, y 220-480.
  - NORMAL EXPLOSION (BIG): 2-3 rows of 10-12 frames, y 500-800.
  - GRENADE EXPLOSION (NORMAL): 12-14 frame rise (y 800-870) and 16-18 frame big rows (y 880-1450, four rows of 9).
  - GRENADE EXPLOSION (BIG) (on water): 3 rows x 8, y 1450-1660 (plus dark-grey smoke rows).
  - MINE EXPLOSIONS: 2 rows x 14 (circle rings, y 1670-1880).
  - FIRE BOMB EXPLOSION (BIG) (in the air): 3 rows x 9-10, y 1950-2450.
  - FIRE BOMB EXPLOSION (BIG) (on ground): 3 rows x 9 + 4-frame loop, y 2460-2560.
  - FIRE EFFECT (used in MS1 when a rebel soldier is hit by a flame shot): 1 row of 12 small frames, y 2560-2600 plus 6 large frames, y 2600-2730.
  - BIG EXPLOSION (on water): 3 rows x 7-8, y 2830-3050.
  - HUGE EXPLOSION AHEAD: 3 rows x 8, y 3140-3357 (exclude label).
- Exclude: credits box (y 3050-3357, x 0-592 bottom-right block), all section labels (ALL-CAPS text lines), the "fire effect" caption.
- Baked: Explosion_Small (28), Explosion_Mid (28), Explosion_Large (28), Explosion_Huge (24), Mini_Explosion (28), Enemy_Bomb_Exp (8), PlayerBombExplosion (27).
  Gaps: fire-bomb explosion (air and ground), big explosion on water, grenade big, mine ring. Cut target for bomb/water explosions.

### 2.16 39079.png (items)
- Size 819x1917 RGBA. Key: alpha (75.1% transparent). Top-left column (x 0-819, y 0-959) and bottom column (y 959-1917).
- Sections (approx sheet y): "Useful items" (y 0-270: arms icons, capsules, ammo capsules FIRE/AP/BOMB/NORM/CLOUD), "Medicine" (y 300-410), "Food" (y 430-820: bananas, carrots, apples, watermelon, cabbage, bread, chicken, rice), "Animals" (y 880-950), "Other items" (y 1040-1800), zantetsu/thunder shot icons (y 130-290, in the Useful items column).
- Exclude: credits banner (approx y 1800-1900, "ITEMS FROM THE METAL SLUG SERIES", ripper MAGMA DRAGON).
- Baked: Item_Apple (29), Item_Banana (8), Item_Banana_Bundle (16), Item_Bread (1), Item_Cabbage (16), Item_Carrot (16), Item_Chicken (12), Item_Fish (6), Item_Fruits (1), Item_Letter (13), Item_Mando_Begin/Idle, Item_Pair (16), Item_Watermelon (16). Most food is covered.
  Sheet adds: medicine bottles, animals (pig/rabbit/fish), weapon capsule icons (letters), ammo capsules (FIRE/AP/BOMB/NORM/CLOUD). Mostly single-frame icons. Low-mid priority.

### 2.17 35366.png (Mission 4 objects: boat, plane)
- Size 320x280 RGBA. Key: alpha (72.8% transparent). Label "Boat", "Plane", credits text.
- Contents: boat intact (approx x 10-150, y 25-140, 1 frame), boat wrecked (approx x 165-300, y 45-150, 1 frame), plane (2 frames at x 10-90 and 90-160, y 180-210).
- Exclude: labels "Boat", "Plane" and the credit text (y 225-280).
- Baked: Mob_Boat_Idle, Mob_Boat_Destroy (EnemySpecs 4.2, 293x248 centre pivot), Plane (2 frames). Duplicates. Skip unless the baked art differs.

### 2.18 20642.png (helicopter / aparch)
- Size 776x765 P (palette, transparent). Background (0,248,0) alpha 0 (66% of pixels). No marker colours.
- Sections (1x, measured by connected components on alpha>0):
  - FLYING: 7 frames, y 10-80 (cell pitch 93-94 px, cells x 6,100,193,287,381,476,567; each 91x57-68).
  - TURNING: 7 frames, y 99-161 (cells x 6,101,190,275,356,435,510; heights 60-62).
  - MISSILE LAUNCHER: 4 tiny launch frames (x 9-70, y 186-208) + 10 launcher frames (x 127-472, y 186-208), about 14 in total.
  - CHOPPER'S BLADE: 7 tilt rows x 4-5 rotor frames, y 228-408 (row y: 228, 256, 286, 311, 335, 356, 379; widths 60-180).
  - MISSILE and BOMB flight: row 1 about 14 missile frames with trail (y 432-450), row 2 about 8 (y 455-473), bombs about 8 (y 478-506).
  - EFFECTS: explosion puffs, about 10 frames (y 524-557, x 11-323), and smoke/debris, about 8 frames (y 564-597).
- Exclude: "FLYING", "TURNING", etc. labels (y 4-19, 84-99, 165-180, 207-222, 412-427, 507-522) and the credits panel and comic (y 600-765).
- Baked Aparch clips: Flight_Left1..7 and Flight_Right1..7 (5 frames each, 70 total), Idle_L/R (1), Move_L/R (6), MoveStop (6), Gun_L/R (9), Gun_End_L/R (9),
  Missle_L/R (2), Missle_Effect_L/R (10), DropMissle (12), DropMissle_Effect (8), Destroy_Left (8), Drop_Bomb_L/R (1). Also Aparch_Turn_Left/Right as a 2828x120 strip sprite.
- Likely overlap: the 7 tilt rows x 5 frames of the blade section match the 7 Aparch_Flight_Left/Right variants (5 frames each). Verify before cutting the blades.
  Clear gaps: Missile flight (about 22 frames with trail) vs baked Aparch_Missle_L/R (2 frames each); Flying (7 frames) and Turning (7 frames) vs baked Aparch_Idle (1 frame).
  Bomb flight (8 frames) vs Aparch_Drop_Bomb (1 frame each). Launcher (about 14) vs Aparch_Gun (9).

### 2.19 32529.gif (red blood effects)
- Size 504x407 P (single frame in the file; the GIF contains one composite sheet). Background alpha 0 (88% pixels).
- Contents: about 6 rows of red splatter sprites with decreasing size, in four bands: top rows (9 + 8 + 9 round splatters, about 26), middle (4 large splatters + 4 smaller, about 8), bottom (5-8 smear frames).
- Exclude: nothing (no labels).
- Baked: no blood clip (no "Blood" names in anim.json). Gap confirmed. Cut now (for hit feedback on enemies).

## 3. Sheets already in sheets.json (state)

| Sheet | State | Clips |
|---|---|---|
| 46546.png | Pilot (duplicate of baked Parker) | Parker_Run, Parker_Stopping, Parker_Run_2 |
| 38098.png | Pilot (duplicate of baked Order_*) | Order_Run_Right (mirror), Order_Stopping, Order_Idle, Order_Recoil, Order_Turn, Order_Run_2 |
| 40204.png | Written (124 clips) | BigShiee_* |
| 29720.png | Partial (37 mission glyphs) | UI_Mission_* |
| 53213.png | Not written | none |
| 33825.png | Not written | none |
| 143506.png | Not written | none |
| 32522.png | Not written | none |

## 4. Prioritized recommendations

### 4.1 Cut now (parity work, clear gaps, verify the mapping first where noted)
1. 51797.png, GAME OVER screen. New art, no baked equivalent. Crop the helmetless panel (x 309-617, y 0-243) and key magenta corners. Export at scale 2.
2. 32529.gif, blood splatter. New effect, no baked equivalent. Use as a hit-feedback clip set (about 26 round splatters + 8 smears).
3. 33824.png, Mission 4 layer re-cut. Cut the Stage0 building layers from the 1x master using the cluster offsets in section 2.14. Scale 2. This addresses misaligned building layers without changing the bake pipeline.
4. 20642.png, helicopter missile flight (22 + 8 bomb) and flying/turning (7 + 7). Clear gaps versus Aparch_Missle_* (2 frames), Aparch_Idle (1 frame). Skip the blades until verified against Aparch_Flight_*.
5. 11302.png, fire-bomb explosion (air and ground), big on water, grenade big, mine. Gaps against Explosion_* and PlayerBombExplosion. Keep the existing S/M/L/Huge.
6. 33825.png, bus Opening + Damaged + Window break + Destroyed. Verify MainTruck mapping first.
7. 53213.png, green tank Driving / Hill / Appearing / Hit. Verify whether this is MissleTank (BTank is a fixed turret per EnemySpecs 2.3).

### 4.2 Next (mid priority)
8. 11279.png, Hyakutaro POW bound/cheer/hit frames for Older variants. Confirm Older mapping first.
9. 29720.png, GO! signs (normal, diagonal, up/down) and CONTINUE digits.
10. 39079.png, medicine and animal items (single-frame icons) if a pickup needs them.

### 4.3 Skip or defer
11. 35366.png (boat, plane) and 140780.png (HUD pieces): duplicates of Mob_Boat_*, Plane, PlayerStateUI_*, TimeNumberUI. Skip unless the baked art differs.
12. 143506.png (title logo): TitleBackground exists. Skip.
13. 16340.png (debris variants) and 20587.png (weapon SFX): low value; 20587 is mostly out-of-scope weapons (FS/RL/EC).
14. 32522.png: unidentified. Do not cut until labelled. Next step: 3x crops of each labelled row.

### 4.4 Exclusion list (never feed these areas to `auto`)
- 11279.png: y 0-15, 525-535, 721-729, 1033-1040, 1540-1550, 2440-2448, credits y >= 2466.
- 20642.png: label rows (y 4-19, 84-99, 165-180, 207-222, 412-427, 507-522), credits y >= 600.
- 16340.png: credit text (0,1488)-(330,1550).
- 20587.png: example panel (y 3620-6200), credits (0,6740)-(605,6950).
- 11302.png: credits (0,3050)-(592,3357); labels.
- 33825.png: green marker colour (0,251,0), credits y 1560-1700.
- 33824.png: Mission 4 credit (6036,675)-(6810,712).
- 39079.png: credits banner (0,1800)-(819,1917).
- 29720.png: credits and MISSION START banner (y 1060-1357).
- 140780.png: credit text (0,112)-(97,131).
- 35366.png: labels and credit text (y 225-280).
- 143506.png: caption text (top) and SUPERVEHICLE labels.

### 4.5 Open questions to verify in-game before cutting
- Which tank is the green tank (53213): MissleTank or a new enemy?
- Which POW is Older (baked): Hyakutaro (11279) or another character?
- Do the Stage0 layer misalignments match the cluster offsets in 2.14? Check in-game before editing sheets.json.
- Is the 7x5 blade overlay in 20642 already covered by Aparch_Flight_*?
