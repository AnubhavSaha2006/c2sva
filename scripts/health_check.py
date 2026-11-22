"""Health check of the frozen re-run (runs/F*), compared with the earlier campaign (runs/E*).

Prints "OK ..." or one "ALERT ..." line per problem:
  * a lane unit failed / stopped before writing 'lane X done'
  * a Python traceback or c2sva crash in a lane log
  * no progress (new case lines or stage markers) for STALL minutes while a lane is active
  * low free memory
  * an ORIGINAL whose outcome got worse than in the E-run (pass/bounded -> fail/error), or any
    failing original (expected: none), or an error status that the E-run did not have
  * prove/qed errors recorded in prove.json / qed.json
State (last progress time) is kept in runs/health_state.json.
"""
import glob
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "src"))
from c2sva import pipeline  # noqa: E402

STALL = 120   # minutes
alerts, notes = [], []
runs = ROOT / "runs"


def unit_state(u):
    return subprocess.run(["systemctl", "--user", "is-active", u], capture_output=True, text=True).stdout.strip()


# 1. lanes
for lane in ("A", "B"):
    log = runs / f"frozen_{lane}.log"
    st = unit_state(f"c2sva-frozen-{lane}")
    text = log.read_text(errors="replace") if log.exists() else ""
    done = f"lane {lane} done" in text
    if st not in ("active", "activating") and not done:
        alerts.append(f"lane {lane} unit is '{st}' but the lane did not finish")
    if re.search(r"Traceback \(most recent call last\)", text):
        tb = text[text.rfind("Traceback"):][:400].replace("\n", " | ")
        alerts.append(f"lane {lane} log contains a Python traceback: {tb}")
    notes.append(f"lane {lane}: {st}{' (done)' if done else ''}, stage: {(re.findall(r'=== .*', text) or ['-'])[-1][4:]}")

# 2. progress / stall
state_f = runs / "health_state.json"
state = json.loads(state_f.read_text()) if state_f.exists() else {}
sig = sum(len(re.findall(r"^\[\d+/\d+\]|^=== |unbounded=", (runs / f"frozen_{l}.log").read_text(errors="replace"), re.M))
          for l in ("A", "B") if (runs / f"frozen_{l}.log").exists())
sig += len(glob.glob(str(runs / "F*/cases/**/prove.json"), recursive=True))
now = time.time()
if sig != state.get("sig"):
    state.update(sig=sig, t=now)
elif any(unit_state(f"c2sva-frozen-{l}") == "active" for l in ("A", "B")):
    idle = (now - state.get("t", now)) / 60
    if idle > STALL:
        alerts.append(f"no progress for {idle:.0f} min while a lane is active")
state_f.write_text(json.dumps(state))

# 3. memory
mem = {l.split(":")[0]: int(l.split()[1]) for l in open("/proc/meminfo") if l.split(":")[0] in ("MemAvailable",)}
if mem.get("MemAvailable", 1 << 30) < 800_000:
    alerts.append(f"low memory: {mem['MemAvailable'] // 1024} MB available")

# 4. verdicts of originals vs the earlier campaign
pairs = {"F1_conformance": "E1_conformance", "F3_abe": "E3_abe", "F4_acslbench": "E4_acslbench",
         "F2_conformance_cfg": "E2_conformance_cfg", "F5_abe_cfg": "E5_abe_cfg",
         "F6_conformance_N2": "E6_conformance_N2", "F6_conformance_N8": "E6_conformance_N8"}
rank = lambda o: 0 if o.startswith("pass") else 1 if o.startswith("bounded") or o == "vacuous" else 2
checked = 0
for frun, erun in pairs.items():
    for f in glob.glob(str(runs / frun / "cases/*/*/*/original/result.json")):
        r = json.loads(Path(f).read_text())
        o = pipeline.outcome(r)
        checked += 1
        rel = Path(f).relative_to(runs / frun)
        old = runs / erun / rel
        o_old = pipeline.outcome(json.loads(old.read_text())) if old.exists() else None
        if o.startswith("fail"):
            alerts.append(f"{frun}: failing ORIGINAL {r['design']} [{r['config']}] {o} replay={(r.get('replay') or {}).get('class')} (E-run: {o_old})")
        elif o == "error" and o_old not in ("error", None, "unsupported"):
            alerts.append(f"{frun}: new ERROR on {r['design']} [{r['config']}]: {str(r.get('error'))[:150]} (E-run: {o_old})")
        elif o_old and rank(o) > rank(o_old) and not (o.startswith("bounded") and o_old.startswith("bounded")):
            notes.append(f"{frun}: {r['design']} [{r['config']}] {o_old} -> {o}")
for f in glob.glob(str(runs / "F*/cases/**/prove.json"), recursive=True) + glob.glob(str(runs / "F*/cases/**/qed.json"), recursive=True):
    o = json.loads(Path(f).read_text())
    if o.get("error") or o.get("status") == "ERROR":
        alerts.append(f"prove/qed error in {Path(f).parent.relative_to(runs)}: {str(o.get('error') or o.get('detail'))[:150]}")

ack = (runs / "health_ack.txt").read_text().split("\n") if (runs / "health_ack.txt").exists() else []
ack = [a.strip() for a in ack if a.strip() and not a.startswith("#")]
acked = [a for a in alerts if any(p in a for p in ack)]
alerts = [a for a in alerts if a not in acked]
notes += [f"acknowledged: {a[:120]}" for a in acked]
print(("ALERT" if alerts else "OK") + f" {time.strftime('%F %T')} originals checked: {checked}")
for a in alerts[:25]:
    print("ALERT", a)
for n in notes[:25]:
    print("note", n)
