"""SymbiYosys back end.

Tasks (one .sby file):
  bmc     functional properties (assumptions + postconditions + frame conditions)
  access  same harness with -DCHECK_ACCESS (memory accesses within \\valid ranges)
  cover   non-vacuity: a call completes under the assumptions
  lat     latency search on harness_lat.sv (sequential calls, termination only)

Latency search: Vitis reports a latency only for fixed trip counts. We instead establish a
bound L by BMC on the termination property alone, starting from the report (capped) and
doubling until it holds or the cap is reached. L is used only as the BMC depth of the other
tasks (no property contains a cycle count). A design whose calls do not finish within the cap
is reported as 'termination not established'.

Solver portfolio: each task runs with the first engine; on timeout it is retried with the next.
A timeout still reports how many steps were checked without a violation ('bounded to k').
"""
import os
import re
import resource
import signal
import subprocess
import time
from pathlib import Path

MEM_LIMIT = int(3.5 * (1 << 30))     # per formal process tree member; a run out of memory = 'bounded'


def _limit_memory(limit):
    def f():
        resource.setrlimit(resource.RLIMIT_AS, (limit, limit))
    return f

ENGINES = {"yices": "smtbmc yices", "bitwuzla": "smtbmc bitwuzla", "boolector": "smtbmc boolector",
           "abc": "abc bmc3"}


POWER_ON = "fpga"   # "fpga": DUT registers without an initial value start at 0 (FPGA INIT, as in
                    # Vitis co-simulation); "arbitrary": any power-on value (ASIC-style, stricter)


def power_on_cmd(rtl_files):
    """Yosys command fixing the power-on value of DUT registers without an explicit initial
    value (harness state is not affected: only the DUT modules are selected)."""
    if POWER_ON != "fpga":
        return ""
    mods = " ".join(Path(f).stem for f in rtl_files)
    return f"setundef -zero -init {mods}\n"


def sby_text(rtl_files, depth, engine, lat_depth=None):
    reads = "\n".join(f"read -formal {Path(f).name}" for f in rtl_files)
    files = "\n".join(str(f) for f in rtl_files)
    return f"""[tasks]
bmc
access
phys
cover
lat

[options]
bmc: mode bmc
bmc: depth {depth}
access: mode bmc
access: depth {depth}
phys: mode bmc
phys: depth {depth}
cover: mode cover
cover: depth {depth}
lat: mode bmc
lat: depth {lat_depth or depth}

[engines]
{ENGINES[engine]}

[script]
{reads}
bmc: read -formal harness.sv
cover: read -formal harness.sv
access: read -formal -DCHECK_ACCESS -DC2SVA_ACCESS_ONLY harness_acc.sv
phys: read -formal -DCHECK_PHYS -DC2SVA_ACCESS_ONLY harness_acc.sv
lat: read -formal harness_lat.sv
prep -top harness
{power_on_cmd(rtl_files)}
[files]
{files}
harness.sv
harness_acc.sv
harness_lat.sv
"""


def write_sby(fdir, rtl_files, depth, engine, lat_depth=None):
    (fdir / "check.sby").write_text(sby_text(rtl_files, depth, engine, lat_depth))


def _steps_checked(fdir, task):
    """task: a task name (check_<task>) or an engine-qualified work dir name (<eng>_<task>)."""
    log = fdir / f"check_{task}" / "logfile.txt"
    if not log.exists():
        return None
    steps = re.findall(r"Checking (?:assertions|cover reachability) in step (\d+)", log.read_text(errors="replace"))
    return int(steps[-1]) if steps else None


def _parse(fdir, task, out, lines, secs, workdir):
    (fdir / f"sby_{task}.log").write_text(out)
    status = "PASS" if "DONE (PASS" in out else "FAIL" if "DONE (FAIL" in out else "ERROR"
    if status == "ERROR" and re.search(r"bad_alloc|MemoryError|[Oo]ut of memory|Cannot allocate|std::bad|returned 13[47]|Killed|returncode=-9", out):
        return "TIMEOUT", ["memout"], secs, _steps_checked(fdir, workdir)
    failed = sorted({lines.get(int(n), f"line {n}")
                     for n in re.findall(r"Assert failed in harness: harness(?:_\w+)?\.sv:(\d+)\.", out)})
    step = re.findall(r"(?:failed assertion|reached cover statement).*? step (\d+)", out)
    if status == "ERROR":
        failed = [" | ".join(l for l in out.splitlines() if "ERROR" in l)[:300]]
    return status, failed, secs, (int(step[0]) if step else None)


def _start(fdir, sbyfile, task, out=subprocess.PIPE):
    # own process group: on timeout only this case's solvers are killed (a global `pkill -f
    # check_<task>/` also killed other cases' solvers when jobs run in parallel)
    return subprocess.Popen(["sby", "-f", sbyfile, task], cwd=fdir, stdout=out,
                            stderr=subprocess.STDOUT, text=True, start_new_session=True,
                            preexec_fn=_limit_memory(MEM_LIMIT))


def _kill(p):
    try:
        os.killpg(p.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass
    if p.stdout:
        p.communicate()
    else:
        p.wait()


def run_task(fdir: Path, task, lines, timeout):
    """-> (status, failed ids, seconds, step)  status: PASS | FAIL | ERROR | TIMEOUT"""
    t0 = time.time()
    p = _start(fdir, "check.sby", task)
    try:
        out, _ = p.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        _kill(p)
        return "TIMEOUT", [], round(time.time() - t0, 1), _steps_checked(fdir, task)
    return _parse(fdir, task, out, lines, round(time.time() - t0, 1), task)


STAGGER = 90      # s before the second engine joins (most tasks finish well before)


def run_portfolio(fdir, rtl, depth, lat_depth, engines, task, lines, timeout):
    """Run `task` with the engines in parallel (the second one joins after STAGGER s); the first
    conclusive answer (PASS/FAIL) wins. -> (status, failed, wall secs, step, engine).
    The winning work directory is linked as check_<task> (replay reads the trace there)."""
    if len(engines) == 1:
        write_sby(fdir, rtl, depth, engines[0], lat_depth)
        st, failed, secs, step = run_task(fdir, task, lines, timeout)
        return st, failed, secs, step, engines[0]
    t0 = time.time()
    procs, results = {}, {}
    for eng in engines:
        (fdir / f"check_{eng}.sby").write_text(sby_text(rtl, depth, eng, lat_depth))
    pending = list(engines)
    started = {}
    while True:
        now = time.time() - t0
        if pending and (not procs or now >= STAGGER * len(procs)):
            eng = pending.pop(0)
            fh = open(fdir / f"sby_{eng}_{task}.out", "w")     # a file, not a pipe (no deadlock)
            procs[eng], started[eng] = _start(fdir, f"check_{eng}.sby", task, out=fh), time.time()
            fh.close()
        for eng, p in list(procs.items()):
            if p.poll() is not None:
                out = (fdir / f"sby_{eng}_{task}.out").read_text(errors="replace")
                secs = round(time.time() - started[eng], 1)
                results[eng] = _parse(fdir, task, out, lines, secs, f"{eng}_{task}")
                del procs[eng]
            elif time.time() - started[eng] > timeout:
                _kill(p)
                results[eng] = ("TIMEOUT", [], round(time.time() - started[eng], 1),
                                _steps_checked(fdir, f"{eng}_{task}"))
                del procs[eng]
        win = next((e for e in engines if e in results and results[e][0] in ("PASS", "FAIL")), None)
        if win or (not procs and not pending):
            for p in procs.values():
                _kill(p)
            if not win:      # all inconclusive: report the one that got furthest
                win = max(results, key=lambda e: (results[e][0] != "TIMEOUT", results[e][3] or -1))
            link = fdir / f"check_{task}"
            if link.is_symlink() or link.is_file():
                link.unlink()
            elif link.exists():
                import shutil
                shutil.rmtree(link)
            if (fdir / f"check_{win}_{task}").exists():
                link.symlink_to(f"check_{win}_{task}")
            st, failed, _, step = results[win]
            return st, failed, round(time.time() - t0, 1), step, win
        time.sleep(0.5)


def discover_latency(fdir, rtl, regen_lat, start, cap, engines, timeout):
    """regen_lat(L) writes harness_lat.sv for bound L. -> (L, status)"""
    L = max(2, min(start, cap))
    while True:
        regen_lat(L)
        st, failed, _, _, _ = run_portfolio(fdir, rtl, L + 4, L + 3, engines, "lat", {}, timeout)
        if st == "PASS":
            return L, "established"
        if st == "TIMEOUT":
            return L, "search timeout"
        if st != "FAIL":
            return L, f"latency search error: {'; '.join(failed)[:200]}"
        if L >= cap:
            return L, "not established within cap"
        L = min(cap, L * 2)
