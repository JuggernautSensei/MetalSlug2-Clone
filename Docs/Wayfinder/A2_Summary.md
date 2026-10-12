# A2 Usage Summary (Resources, Assets/Source, Assets/Defs, Assets/Stage, Assets/Tuning)
Total files: 1670 (1609.28 MB). Full list: Docs/Wayfinder/A2_Usage.csv.
Method: refs = sheets/SoundMap/GroundEdits, Assets/Defs, Stage, Tuning, Source *.cpp|h, Scripts/*, Resources/ResourceData/*.
Match: basename (stem for .png/.gif/.bmp/.anime), case-insensitive, token-bounded; then scope-relative path. Baker globs *.anime/*.sprite count as USED.
MAYBE: no name match, but code has a dir literal 'dir/' for that folder (dynamic load possible).

| Folder | USED files / MB | UNUSED files / MB | MAYBE files / MB | Total files / MB |
|---|---|---|---|---|
| Resources | 979 / 609.79 | 218 / 56.23 | 79 / 122.74 | 1276 / 788.76 |
| Assets/Source | 201 / 10.48 | 183 / 809.94 | 0 / 0.00 | 384 / 820.42 |
| Assets/Defs | 6 / 0.06 | 0 / 0.00 | 0 / 0.00 | 6 / 0.06 |
| Assets/Stage | 3 / 0.04 | 0 / 0.00 | 0 / 0.00 | 3 / 0.04 |
| Assets/Tuning | 1 / 0.01 | 0 / 0.00 | 0 / 0.00 | 1 / 0.01 |
| **All** | 1190 / 620.37 | 401 / 866.17 | 79 / 122.74 | 1670 / 1609.28 |
Notes: UNUSED = no reference found in any source listed above. Verify MAYBE rows before trashing.
