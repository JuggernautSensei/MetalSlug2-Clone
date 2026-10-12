#!/usr/bin/env python3
"""sheets.json 의 40204.png 항목만 GenBossSheet.py 의 현재 결과로 바꾼다 (다른 시트 항목의 서식은 건드리지 않는다).

사용: python3 Scripts/ReplaceBossSheet.py
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import GenBossSheet as G

ROOT = G.ROOT
PATH = os.path.join(ROOT, "Assets", "Source", "sheets.json")


def block_text():
    clips = G.build()
    lines = []
    for c in clips:
        frames = json.dumps(c["frames"], separators=(",", ":")).replace("],[", "], [")
        parts = ['"name": "%s"' % c["name"], '"frames": %s' % frames, '"pivot": "%s"' % c["pivot"], '"fps": %d' % c["fps"], '"loop": %s' % ("true" if c["loop"] else "false")]
        if "origins" in c:
            parts.append('"origins": %s' % json.dumps(c["origins"], separators=(",", ":")).replace("],[", "], ["))
        if "register" in c:
            parts.append('"register": %s' % ("true" if c["register"] else "false"))
        lines.append("                { " + ", ".join(parts) + " }")
    return (
        "        {\n"
        '            "file": "40204.png",\n'
        '            "scale": 2,\n'
        '            "noRegisterPrefixes": ' + json.dumps(G.NOREG) + ",\n"
        '            "key": { "mode": "color", "colors": [[153, 217, 234], [0, 255, 0]], "tol": 0 },\n'
        '            "clips": [\n' + ",\n".join(lines) + "\n"
        "            ]\n"
        "        }"
    )


def main():
    with open(PATH, encoding="utf-8", newline="") as f:
        text = f.read()
    crlf = "\r\n" in text
    text = text.replace("\r\n", "\n")
    marker = '        {\n            "file": "40204.png"'
    start = text.find(marker)
    if start < 0:
        sys.exit("40204.png 항목을 찾지 못했다")
    depth = 0
    end = -1
    for i in range(start, len(text)):
        ch = text[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    if end < 0:
        sys.exit("항목 끝을 찾지 못했다")
    text = text[:start] + block_text() + text[end:]
    json.loads(text)
    if crlf:
        text = text.replace("\n", "\r\n")
    with open(PATH, "w", encoding="utf-8", newline="") as f:
        f.write(text)
    print("replaced 40204 block")


if __name__ == "__main__":
    main()
