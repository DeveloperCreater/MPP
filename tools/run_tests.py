import argparse
import subprocess
import sys
from pathlib import Path


def norm_output(s: str) -> str:
    # Normalize newlines and trailing whitespace for stable comparisons.
    # Also strip any UTF-8 BOM that might appear (common on Windows).
    if s.startswith("\ufeff"):
        s = s.lstrip("\ufeff")
    s = s.replace("\r\n", "\n").replace("\r", "\n")
    return "\n".join(line.rstrip() for line in s.split("\n")).strip() + "\n"


def run_one(case_path: Path, runner: list[str]) -> tuple[int, str, str]:
    # runner is a command prefix like: ["python", "run.py"] or ["dist\\mpp.exe", "-r"]
    cmd = runner + [str(case_path)]
    p = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    return p.returncode, p.stdout, p.stderr


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cases", default="tests/cases", help="Folder with .mpp + .out files")
    ap.add_argument(
        "--runner",
        default="python run.py",
        help="Command prefix used to run a file. Examples: \"python run.py\" or \"dist\\\\mpp.exe -r\"",
    )
    ap.add_argument(
        "--include-native",
        action="store_true",
        help="Also run *.native.mpp cases (requires a native-capable runner)",
    )
    args = ap.parse_args()

    cases_dir = Path(args.cases)
    if not cases_dir.exists():
        print(f"Missing cases dir: {cases_dir}", file=sys.stderr)
        return 2

    runner = args.runner.split(" ")
    mpps = sorted(cases_dir.glob("*.mpp"))
    if not args.include_native:
        mpps = [p for p in mpps if not p.name.endswith(".native.mpp")]

    total = 0
    failed = 0

    for mpp in mpps:
        total += 1
        exp = mpp.with_suffix(".out")
        if not exp.exists():
            print(f"[SKIP] {mpp.name} (missing {exp.name})")
            continue

        rc, out, err = run_one(mpp, runner)
        got = norm_output(out)
        want = norm_output(exp.read_text(encoding="utf-8-sig", errors="replace"))

        if rc != 0:
            failed += 1
            print(f"[FAIL] {mpp.name} (exit {rc})")
            if out.strip():
                print("--- stdout ---")
                print(out)
            if err.strip():
                print("--- stderr ---")
                print(err)
            continue

        if got != want:
            failed += 1
            print(f"[FAIL] {mpp.name} (output mismatch)")
            print("--- want ---")
            print(want.encode("utf-8", errors="replace").decode("utf-8"))
            print("--- got ---")
            print(got.encode("utf-8", errors="replace").decode("utf-8"))
            continue

        print(f"[OK]   {mpp.name}")

    print(f"\nResult: {total - failed}/{total} passed")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
