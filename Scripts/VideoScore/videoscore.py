"""VideoScore: score the remake against the original arcade footage per named scene.

One command (from the repo root):
  python Scripts/VideoScore/videoscore.py                      # all scenes, runs the remake (offscreen), writes Captures/VideoScore/<stamp>/
  python Scripts/VideoScore/videoscore.py --scene cannon_deploy --reuse Captures/VideoScore/last
Options: --exe <path> (default Output/Release/MetalSlug2_Clone.exe), --out <dir>, --reuse <dir with runs/>, --scene <name> (repeatable),
         --cwd <dir> (asset root parent; default repo root), --list.
Exit code: 0 if every scored scene >= passThreshold, 1 otherwise, 2 on setup error.
Docs: Docs/Wayfinder/Scoring.md. Never writes into Assets/ or Source/.
"""
import argparse
import datetime
import glob
import json
import math
import os
import shutil
import subprocess
import sys

import cv2
import numpy as np

sys.path.insert(0, os.path.dirname(__file__))
import sprites  # noqa: E402

ROOT = sprites.ROOT


# ---------------------------------------------------------------- images

HUD = {"top": 0, "bottom": 0}


def grad(gray, hud=False):
    g = cv2.GaussianBlur(gray.astype(np.float32), (0, 0), 1.5)
    m = cv2.magnitude(cv2.Sobel(g, cv2.CV_32F, 1, 0), cv2.Sobel(g, cv2.CV_32F, 0, 1))
    if hud:
        m[:HUD["top"]] = 0
        if HUD["bottom"]:
            m[m.shape[0] - HUD["bottom"]:] = 0
    return m


_tpl_cache = {}


def template(spec, sub=None):
    """spec 'Clip:frame' or 'sprite:Name' -> (gradient template, pivot offset inside the template) or None."""
    key = (spec, tuple(sub) if sub else None)
    if key in _tpl_cache:
        return _tpl_cache[key]
    if spec.startswith("sprite:"):
        with open(os.path.join(sprites.ASSETS, "sprites.json"), encoding="utf-8") as f:
            s = json.load(f)["sprites"].get(spec[7:])
        if not s:
            _tpl_cache[key] = None
            return None
        im = sprites.page(s["atlas"])[s["y"]:s["y"] + s["h"], s["x"]:s["x"] + s["w"]]
        piv = (s.get("pivotX", 0), s.get("pivotY", 0))
    else:
        clip, _, idx = spec.partition(":")
        im, piv = sprites.frame(clip, int(idx or 0))
        if im is None:
            _tpl_cache[key] = None
            return None
    if im.shape[2] == 3:
        im = cv2.cvtColor(im, cv2.COLOR_BGR2BGRA)
    x0 = y0 = 0
    if sub:
        h, w = im.shape[:2]
        x0, y0, x1, y1 = int(sub[0] * w), int(sub[1] * h), int(sub[2] * w), int(sub[3] * h)
        im = im[y0:y1, x0:x1]
    g = grad(cv2.cvtColor(im[:, :, :3], cv2.COLOR_BGR2GRAY))
    m = cv2.erode((im[:, :, 3] > 128).astype(np.uint8), np.ones((3, 3), np.uint8))
    out = (g * m, (piv[0] - x0, piv[1] - y0))
    _tpl_cache[key] = out
    return out


def _rotated(spec, sub, deg):
    key = (spec, tuple(sub) if sub else None, "rot", deg)
    if key not in _tpl_cache:
        base = template(spec, sub)
        if base is None or deg == 0:
            _tpl_cache[key] = base
        else:
            g, piv = base
            h, w = g.shape
            m = cv2.getRotationMatrix2D((piv[0], piv[1]), deg, 1.0)
            c, s_ = abs(m[0, 0]), abs(m[0, 1])
            nw, nh = int(h * s_ + w * c) + 2, int(h * c + w * s_) + 2
            m[0, 2] += nw / 2 - piv[0]
            m[1, 2] += nh / 2 - piv[1]
            _tpl_cache[key] = (cv2.warpAffine(g, m, (nw, nh)), (nw / 2, nh / 2))
    return _tpl_cache[key]


def locate(frame_grad, spec, sub=None, rot=None):
    """Best match -> (confidence, (x, y) pivot position in frame px) or (0, None). Partly off-screen sprites still match (padded frame).
    rot: list of template rotations in degrees (sprites that the game rotates, e.g. Boss_Missile3)."""
    best = (0.0, None)
    for deg in (rot or [0]):
        t = _rotated(spec, sub, deg)
        if t is None:
            return 0.0, None
        g, piv = t
        ph, pw = g.shape[0] // 2, g.shape[1] // 2
        fg = cv2.copyMakeBorder(frame_grad, ph, ph, pw, pw, cv2.BORDER_CONSTANT, value=0)
        if fg.shape[0] < g.shape[0] or fg.shape[1] < g.shape[1]:
            continue
        r = cv2.matchTemplate(fg, g, cv2.TM_CCOEFF_NORMED)
        r[~np.isfinite(r)] = 0
        r[r > 1.0001] = 0
        # reject flat regions (degenerate correlation): local gradient energy must be >= 15% of the template's
        mask = (g > 0).astype(np.float32)
        energy = cv2.matchTemplate(fg * fg, mask, cv2.TM_CCORR) / max(1.0, float(mask.sum()))
        r[energy < 0.15 * float((g * g).sum()) / max(1.0, float(mask.sum()))] = 0
        _, mx, _, loc = cv2.minMaxLoc(r)
        if mx > best[0]:
            best = (float(mx), (int(loc[0] + piv[0] - pw), int(loc[1] + piv[1] - ph)))
    return best


def ssim_small(a, b):
    a = cv2.resize(cv2.GaussianBlur(a, (0, 0), 1.0), (152, 112), interpolation=cv2.INTER_AREA).astype(np.float64)
    b = cv2.resize(cv2.GaussianBlur(b, (0, 0), 1.0), (152, 112), interpolation=cv2.INTER_AREA).astype(np.float64)
    c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
    vals = []
    for j in range(0, 112 - 7, 8):
        for i in range(0, 152 - 7, 8):
            p, q = a[j:j + 8, i:i + 8], b[j:j + 8, i:i + 8]
            mp, mq = p.mean(), q.mean()
            cv_ = ((p - mp) * (q - mq)).mean()
            vals.append(((2 * mp * mq + c1) * (2 * cv_ + c2)) / ((mp * mp + mq * mq + c1) * (p.var() + q.var() + c2)))
    return float(np.mean(vals))


def edge_f1(a, b):
    a = cv2.resize(a, (304, 224), interpolation=cv2.INTER_AREA)
    b = cv2.resize(b, (304, 224), interpolation=cv2.INTER_AREA)
    ea = cv2.Canny(cv2.GaussianBlur(a, (0, 0), 1.2), 40, 110) > 0
    eb = cv2.Canny(cv2.GaussianBlur(b, (0, 0), 1.2), 40, 110) > 0
    k = np.ones((5, 5), np.uint8)
    da = cv2.dilate(ea.astype(np.uint8), k) > 0
    db = cv2.dilate(eb.astype(np.uint8), k) > 0
    if ea.sum() == 0 or eb.sum() == 0:
        return 0.0
    prec = (eb & da).sum() / eb.sum()
    rec = (ea & db).sum() / ea.sum()
    return float(2 * prec * rec / max(1e-6, prec + rec))


def frame_similarity(ref_bgr, rem_bgr, hud):
    a = cv2.cvtColor(ref_bgr, cv2.COLOR_BGR2GRAY)
    b = cv2.cvtColor(rem_bgr, cv2.COLOR_BGR2GRAY)
    a, b = a[hud["top"]:a.shape[0] - hud["bottom"]], b[hud["top"]:b.shape[0] - hud["bottom"]]
    s = max(0.0, ssim_small(a, b))
    e = edge_f1(a, b)
    return 0.5 * s + 0.5 * e, s, e


# ---------------------------------------------------------------- sources

class RefVideo:
    def __init__(self, cfg, size):
        self.path = os.path.join(ROOT, cfg["path"])
        self.crop = cfg["crop"]
        self.size = tuple(size)
        self.cap = cv2.VideoCapture(self.path)
        if not self.cap.isOpened():
            raise SystemExit("cannot open " + self.path)
        self.fps = self.cap.get(cv2.CAP_PROP_FPS)
        self.cache = {}

    def frame(self, t):
        n = int(round(t * self.fps))
        if n not in self.cache:
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, n)
            ok, f = self.cap.read()
            if not ok:
                return None
            x, y, w, h = self.crop
            self.cache[n] = cv2.resize(f[y:y + h, x:x + w], self.size, interpolation=cv2.INTER_LINEAR)
        return self.cache[n]


class RemakeRun:
    """Offscreen capture of one game run (--capture-ticks), stored as PNG under <dir>/runs/<name>/."""

    def __init__(self, name, cfg, run_dir, crop):
        self.name, self.cfg, self.dir, self.crop = name, cfg, run_dir, crop
        self.ticks = list(range(cfg["ticks"][0] + cfg["step"], cfg["ticks"][1] + 1, cfg["step"]))
        self._grad = {}

    def exists(self):
        return len(glob.glob(os.path.join(self.dir, "f_*.png"))) >= len(self.ticks) * 0.9

    def capture(self, exe, cwd):
        os.makedirs(self.dir, exist_ok=True)
        env = dict(os.environ, SDL_VIDEO_DRIVER="offscreen", SDL_RENDER_DRIVER="software")
        prefix = os.path.join(self.dir, "cap")
        args = [exe] + self.cfg["args"] + ["--capture", prefix, "--capture-ticks", ",".join(str(t) for t in self.ticks)]
        if self.cfg.get("eventLog"):
            # T0 (Scoring.md): the game writes one JSON object per line; 'log' anchors read it
            args += ["--event-log", os.path.join(self.dir, "events.jsonl")]
        print("[run]", self.name, " ".join(args[1:5]), "... %d frames" % len(self.ticks), flush=True)
        timeout = 120 + self.cfg["ticks"][1] / 60 * 3
        subprocess.run(args, cwd=cwd, env=env, timeout=timeout, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        for bmp in glob.glob(prefix + "_*.bmp"):
            tick = int(bmp[-9:-4])
            im = cv2.imread(bmp)
            if im is not None:
                cv2.imwrite(os.path.join(self.dir, "f_%05d.png" % tick), im)
            os.remove(bmp)
        if not self.exists():
            raise SystemExit("remake run '%s' produced too few frames (exe/args?)" % self.name)

    def nearest(self, tick):
        return min(self.ticks, key=lambda t: abs(t - tick))

    def frame(self, tick):
        p = os.path.join(self.dir, "f_%05d.png" % self.nearest(tick))
        im = cv2.imread(p)
        if im is None:
            return None
        x0, y0, x1, y1 = self.crop
        return im[y0:y1, x0:x1]

    def frame_full(self, tick):
        return cv2.imread(os.path.join(self.dir, "f_%05d.png" % self.nearest(tick)))

    def gradf(self, tick):
        t = self.nearest(tick)
        if t not in self._grad:
            im = self.frame(t)
            self._grad[t] = None if im is None else grad(cv2.cvtColor(im, cv2.COLOR_BGR2GRAY), True)
        return self._grad[t]


# ---------------------------------------------------------------- detectors

def detect_series(get_grad, keys, det):
    """First key where the detector fires. appear: >= minOn consecutive frames with conf >= thr (returns the first of them);
    vanish: after >= minOn consecutive 'on' frames, the first of >= minOn consecutive 'off' frames."""
    want = max(1, int(det.get("occurrence", 1)))
    min_on = int(det.get("minOn", 2))
    run_on = run_off = hits = 0
    first_on = first_off = None
    armed = False
    for k in keys:
        g = get_grad(k)
        if g is None or float(g.std()) < 1.0:
            continue
        conf = max(locate(g, spec, det.get("sub"), det.get("rot"))[0] for spec in det.get("templates", [det.get("template")]))
        on = conf >= det["thr"]
        if on:
            run_on += 1
            run_off = 0
            first_on = k if run_on == 1 else first_on
            if det["type"] == "appear" and run_on == min_on and not armed:
                hits += 1
                armed = True
                if hits >= want:
                    return first_on
            if det["type"] == "vanish" and run_on >= min_on:
                armed = True
        else:
            run_off += 1
            run_on = 0
            first_off = k if run_off == 1 else first_off
            if det["type"] == "appear" and run_off >= min_on:
                armed = False
            if det["type"] == "vanish" and armed and run_off == min_on:
                hits += 1
                armed = False
                if hits >= want:
                    return first_off
    return None


def log_event_tick(run, det, start_tick=0):
    """Anchor from a game event log (<run>/events.jsonl, one JSON object per line with at least {"tick", "event"}).
    The game does not write this file yet (see Scoring.md, task T0); returns None when it is missing."""
    path = os.path.join(run.dir, "events.jsonl")
    if not os.path.isfile(path):
        return None
    want, hits = int(det.get("occurrence", 1)), 0
    with open(path, encoding="utf-8") as f:
        for line in f:
            try:
                e = json.loads(line)
            except ValueError:
                continue
            if e.get("event") == det["event"] and e.get("tick", 0) >= start_tick and all(e.get(k) == v for k, v in det.get("match", {}).items()):
                hits += 1
                if hits >= want:
                    return int(e["tick"])
    return None


def find_event_tick(run, det, start_tick=0, end_tick=None):
    if det["type"] == "log":
        return log_event_tick(run, det, start_tick)
    if "fallback" in det and log_event_tick(run, det["fallback"]) is not None:
        return log_event_tick(run, det["fallback"])
    start_tick = max(start_tick, int(det.get("startTick", 0)))
    end_tick = run.ticks[-1] if end_tick is None else end_tick
    keys = [t for t in run.ticks if start_tick <= t <= end_tick]
    return detect_series(run.gradf, keys, det)


def find_ref_time(ref, det):
    """Same detector on the original footage (search window det['search'] = [t0, t1] seconds, 1/15 s steps)."""
    t0, t1 = det["search"]
    keys = list(np.arange(t0, t1, 1.0 / 15.0))
    return detect_series(lambda t: grad(cv2.cvtColor(ref.frame(t), cv2.COLOR_BGR2GRAY), True) if ref.frame(t) is not None else None, keys, det)


def scroll_speeds(frames, dt, band):
    """Camera speed (game px/s, + = camera moves right) between frames dt apart, from the ground band. frames: list of (time, bgr)."""
    out = []
    for (t0, a), (t1, b) in zip(frames, frames[1:]):
        ga = cv2.cvtColor(a, cv2.COLOR_BGR2GRAY)[band[0]:band[1]].astype(np.float32)
        gb = cv2.cvtColor(b, cv2.COLOR_BGR2GRAY)[band[0]:band[1]].astype(np.float32)
        x0, x1 = 120, ga.shape[1] - 120
        tpl = ga[:, x0:x1]
        r = cv2.matchTemplate(gb, tpl, cv2.TM_CCOEFF_NORMED)[0]
        # search only -4..+24 px per step (anti-alias: roofs repeat every ~16 game px)
        lo, hi = max(0, x0 - int(30 * (t1 - t0) / 0.5)), min(len(r), x0 + 5)
        j = lo + int(np.argmax(r[lo:hi]))
        out.append((t0, (x0 - j) / (t1 - t0), float(r[j])))
    return out


# ---------------------------------------------------------------- scoring

def score_scene(sc, cfg, refs, runs, outdir):
    res = {"name": sc["name"], "items": sc.get("items", []), "notes": []}
    ref = refs[sc["ref"]["video"]]
    run = runs[sc["remake"]["run"]]
    ref_anchor = sc["ref"]["anchor"]
    if sc["ref"].get("detect"):
        found = find_ref_time(ref, sc["ref"]["detect"])
        res["refAnchorDetected"] = None if found is None else round(float(found), 3)
        if found is not None:
            ref_anchor = float(found)
        else:
            res["notes"].append("ref anchor detector did not fire; using the annotated anchor %.2f" % ref_anchor)
    sc = dict(sc, ref=dict(sc["ref"], anchor=ref_anchor))
    res["refAnchor"] = round(ref_anchor, 3)
    w0, w1 = sc["ref"]["window"]
    anchor_tick = find_event_tick(run, sc["remake"]["anchor"])
    res["remakeAnchorTick"] = anchor_tick
    if anchor_tick is None:
        res["score"] = 0.0
        res["scorable"] = False
        res["notes"].append("remake anchor not found (%s %s): %s" % (sc["remake"]["anchor"]["type"], sc["remake"]["anchor"].get("template", sc["remake"]["anchor"].get("event", sc["remake"]["anchor"].get("templates"))), sc["remake"].get("whyMissing", "feature/event absent in the remake run")))
        sheet(sc, cfg, ref, run, None, outdir, res)
        return res
    res["scorable"] = True

    def rem_tick(t_rel):
        return anchor_tick + int(round(t_rel * 60))

    comps = {}
    # timing
    if sc.get("events"):
        parts, wsum = [], 0.0
        res["events"] = []
        for ev in sc["events"]:
            # search +-max(1.5 s, |ref|) around the expected time; never the anchor frame itself for later events
            span = max(1.5, abs(ev["ref"]))
            start = rem_tick(ev["ref"] - span)
            if ev["ref"] > 0:
                start = max(start, anchor_tick + run.cfg["step"])
            end = rem_tick(ev["ref"] + span)
            t = find_event_tick(run, ev["detector"], max(0, start), end)
            w = ev.get("weight", 1.0)
            if t is None:
                s, dt = 0.0, None
            else:
                dt = (t - anchor_tick) - ev["ref"] * 60
                s = math.exp(-abs(dt) / cfg["tauTicks"])
            res["events"].append({"name": ev["name"], "refTicks": round(ev["ref"] * 60), "remakeTicks": None if t is None else t - anchor_tick, "errTicks": dt, "score": round(s, 3)})
            parts.append(s * w)
            wsum += w
        comps["timing"] = sum(parts) / wsum
    # sample times
    n = max(6, int((w1 - w0) / 0.5) + 1)
    times = list(np.linspace(w0, w1, n))
    # position
    if sc.get("tracks"):
        errs = []
        res["tracks"] = []
        for tr in sc["tracks"]:
            pairs = []
            for tr_t in times:
                rf = ref.frame(ref_anchor + tr_t)
                rg = run.gradf(rem_tick(tr_t))
                if rf is None or rg is None:
                    continue
                rgr = grad(cv2.cvtColor(rf, cv2.COLOR_BGR2GRAY), True)
                c1, p1 = max((locate(rgr, spec, tr.get("sub"), tr.get("rot")) for spec in tr.get("templates", [tr["template"]])), key=lambda x: x[0])
                c2, p2 = max((locate(rg, spec, tr.get("sub"), tr.get("rot")) for spec in tr.get("templates", [tr["template"]])), key=lambda x: x[0])
                if c1 >= tr["thrRef"] and c2 >= tr["thrRem"]:
                    pairs.append((tr_t, p1, p2, math.hypot(p1[0] - p2[0], p1[1] - p2[1])))
            e = float(np.median([p[3] for p in pairs])) if len(pairs) >= 3 else None
            res["tracks"].append({"name": tr["name"], "pairs": len(pairs), "medianErrPx": e,
                                  "samples": [[round(p[0], 2), list(map(int, p[1])), list(map(int, p[2]))] for p in pairs[:12]]})
            if e is not None:
                errs.append(math.exp(-e / cfg["sigmaPx"]))
            else:
                res["notes"].append("track %s: <3 frames where both sides were detected" % tr["name"])
        if errs:
            comps["position"] = float(np.mean(errs))
    # metric (camera scroll replaces position)
    if sc.get("metric", {}).get("type") == "scroll":
        m = sc["metric"]
        step = m["dt"]
        rt = np.arange(w0, w1, step)
        rframes = [(t, ref.frame(ref_anchor + t)) for t in rt]
        mframes = [(t, run.frame(rem_tick(t))) for t in rt]
        rframes = [f for f in rframes if f[1] is not None]
        mframes = [f for f in mframes if f[1] is not None]
        rs = scroll_speeds(rframes, step, m["band"])
        ms = scroll_speeds(mframes, step, m["band"])
        k = min(len(rs), len(ms))
        rv = np.array([v for _, v, _ in rs[:k]])
        mv = np.array([v for _, v, _ in ms[:k]])
        # 2 s moving average: pauses/drives line up loosely
        kern = np.ones(int(2 / step)) / int(2 / step)
        rva, mva = np.convolve(rv, kern, "same"), np.convolve(mv, kern, "same")
        err = float(np.mean(np.abs(rva - mva)))
        mean_ref = float(np.mean(np.abs(rva))) or 1.0
        comps["position"] = max(0.0, 1.0 - err / mean_ref)
        res["scroll"] = {"refMeanPxPerSec": round(float(np.mean(rv)), 1), "remakeMeanPxPerSec": round(float(np.mean(mv)), 1),
                         "refTotalPx": round(float(np.sum(rv) * step)), "remakeTotalPx": round(float(np.sum(mv) * step)),
                         "refPausedFrac": round(float(np.mean(np.abs(rv) < 4)), 2), "remakePausedFrac": round(float(np.mean(np.abs(mv) < 4)), 2),
                         "meanAbsErrPxPerSec": round(err, 1),
                         "curve": [[round(float(t), 1), round(float(a), 1), round(float(b), 1)] for t, a, b in zip(rt[:k], rva, mva)][::2]}
    # frame similarity
    sims = []
    res["frames"] = []
    for t in times:
        rf = ref.frame(ref_anchor + t)
        mf = run.frame(rem_tick(t))
        if rf is None or mf is None:
            continue
        s, ss, ee = frame_similarity(rf, mf, cfg["hudMask"])
        sims.append(s)
        res["frames"].append([round(float(t), 2), round(s, 3), round(ss, 3), round(ee, 3)])
    if sims:
        # normalised by the original-vs-original ceiling (V1 vs V2, same event, different play): cfg frameCeiling
        res["frameRaw"] = round(float(np.mean(sims)), 3)
        comps["frame"] = min(1.0, float(np.mean(sims)) / cfg.get("frameCeiling", 1.0))
    wts = {k: cfg["weights"][k] for k in comps}
    tot = sum(wts.values())
    res["components"] = {k: round(v, 3) for k, v in comps.items()}
    res["score"] = round(sum(comps[k] * wts[k] for k in comps) / tot, 3) if tot else 0.0
    res["pass"] = res["score"] >= cfg["passThreshold"]
    sheet(sc, cfg, ref, run, anchor_tick, outdir, res)
    return res


def sheet(sc, cfg, ref, run, anchor_tick, outdir, res):
    w0, w1 = sc["ref"]["window"]
    times = list(np.linspace(w0, w1, 6))
    cols = []
    for t in times:
        rf = ref.frame(sc["ref"]["anchor"] + t)
        rf = np.zeros((448, 608, 3), np.uint8) if rf is None else rf.copy()
        if anchor_tick is None:
            mf = np.zeros_like(rf)
            label = "remake: anchor missing"
        else:
            tick = run.nearest(anchor_tick + int(round(t * 60)))
            mf = run.frame(tick)
            mf = np.zeros_like(rf) if mf is None else cv2.resize(mf, (rf.shape[1], rf.shape[0]))
            label = "remake tick %d" % tick
        cv2.putText(rf, "orig %s %+.2fs (%.2f)" % (sc["ref"]["video"], t, sc["ref"]["anchor"] + t), (6, 22), 0, 0.6, (0, 0, 0), 4)
        cv2.putText(rf, "orig %s %+.2fs (%.2f)" % (sc["ref"]["video"], t, sc["ref"]["anchor"] + t), (6, 22), 0, 0.6, (0, 255, 255), 1)
        cv2.putText(mf, label, (6, 22), 0, 0.6, (0, 0, 0), 4)
        cv2.putText(mf, label, (6, 22), 0, 0.6, (255, 255, 0), 1)
        cols.append(np.vstack([rf, mf]))
    img = np.hstack(cols)
    img = cv2.resize(img, (img.shape[1] // 2, img.shape[0] // 2), interpolation=cv2.INTER_AREA)
    head = np.zeros((28, img.shape[1], 3), np.uint8)
    cv2.putText(head, "%s  score %.3f  %s" % (sc["name"], res.get("score", 0), json.dumps(res.get("components", {}))), (6, 20), 0, 0.55, (255, 255, 255), 1)
    path = os.path.join(outdir, sc["name"] + "_sheet.png")
    cv2.imwrite(path, np.vstack([head, img]))
    res["sheet"] = os.path.relpath(path, ROOT)


# ---------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scenes", default=os.path.join(os.path.dirname(__file__), "scenes.json"))
    ap.add_argument("--scene", action="append")
    ap.add_argument("--exe", default=os.path.join(ROOT, "Output", "Release", "MetalSlug2_Clone.exe"))
    ap.add_argument("--cwd", default=ROOT)
    ap.add_argument("--out")
    ap.add_argument("--reuse", help="output dir of an earlier run whose runs/ captures are reused")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()
    with open(a.scenes, encoding="utf-8") as f:
        cfg = json.load(f)
    scenes = [s for s in cfg["scenes"] if not a.scene or s["name"] in a.scene]
    if a.list:
        for s in cfg["scenes"]:
            print(s["name"], s.get("items"), s["ref"])
        return 0
    if not scenes:
        print("no such scene")
        return 2
    out = a.out or os.path.join(ROOT, "Captures", "VideoScore", datetime.datetime.now().strftime("%Y%m%d_%H%M%S"))
    os.makedirs(out, exist_ok=True)
    HUD.update(cfg["hudMask"])
    refs = {k: RefVideo(v, cfg["refSize"]) for k, v in cfg["videos"].items()}
    runs = {}
    for name in sorted({s["remake"]["run"] for s in scenes}):
        rd = os.path.join(out, "runs", name)
        run = RemakeRun(name, cfg["runs"][name], rd, cfg["remakeCrop"])
        if a.reuse and not run.exists():
            src = os.path.join(a.reuse, "runs", name)
            if os.path.isdir(src) and os.path.abspath(src) != os.path.abspath(rd):
                shutil.copytree(src, rd, dirs_exist_ok=True)
        if not run.exists():
            if not os.path.isfile(a.exe):
                print("exe not found:", a.exe)
                return 2
            run.capture(a.exe, a.cwd)
        runs[name] = run
    results = []
    for sc in scenes:
        print("[scene]", sc["name"], flush=True)
        r = score_scene(sc, cfg, refs, runs, out)
        with open(os.path.join(out, sc["name"] + ".json"), "w", encoding="utf-8") as f:
            json.dump(r, f, ensure_ascii=False, indent=1)
        results.append(r)
        print("   score %.3f %s %s" % (r["score"], r.get("components", {}), "; ".join(r["notes"])), flush=True)
    summary = {"exe": a.exe, "threshold": cfg["passThreshold"], "scenes": [{k: r.get(k) for k in ("name", "items", "score", "scorable", "components", "notes")} for r in results]}
    with open(os.path.join(out, "summary.json"), "w", encoding="utf-8") as f:
        json.dump(summary, f, ensure_ascii=False, indent=1)
    last = os.path.join(ROOT, "Captures", "VideoScore", "last.txt")
    os.makedirs(os.path.dirname(last), exist_ok=True)
    with open(last, "w", encoding="utf-8") as f:
        f.write(out + "\n")
    print("\n%-18s %6s  %s" % ("scene", "score", "components"))
    for r in results:
        print("%-18s %6.3f  %s %s" % (r["name"], r["score"], r.get("components", {}), "" if r.get("scorable", True) else "(anchor missing)"))
    print("out:", out)
    return 0 if all(r["score"] >= cfg["passThreshold"] for r in results) else 1


if __name__ == "__main__":
    sys.exit(main())
