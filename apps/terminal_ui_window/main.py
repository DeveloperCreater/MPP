import os
import sys
from pathlib import Path
import subprocess

import webview


ROOT = Path(__file__).resolve().parents[2]
WEB_DIR = Path(__file__).resolve().parent / "web"


def run_mpp_expr(code: str) -> str:
    """
    Evaluate M++ code using the Python runtime in this repo (fast dev path).
    We use `python run.py -e "<code>"` so it matches the same behavior as CLI.
    """
    cmd = [sys.executable, str(ROOT / "run.py"), "-e", code]
    p = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    if p.returncode != 0:
        return f"[mpp error]\n{p.stdout}\n{p.stderr}".strip()
    return p.stdout.strip()


class Api:
    def mpp_hello(self, name: str) -> str:
        name = (name or "").replace("\\", "\\\\").replace('"', '\\"')
        # M++ returns strings fine; we use print so result is visible.
        return run_mpp_expr(f'print("Hello, " + "{name}")')

    def mpp_echo(self, text: str) -> str:
        text = (text or "").replace("\\", "\\\\").replace('"', '\\"')
        return run_mpp_expr(f'print("Echo: " + "{text}")')

    def exec_python(self) -> str:
        # Demonstrate calling external language from the app host.
        p = subprocess.run([sys.executable, "-c", "print(6*7)"], capture_output=True, text=True)
        return f"python says: {p.stdout.strip()}"

    def exec_cpp_mpp(self) -> str:
        # Demonstrate C++ integration: if you built cpp\\mpp.exe, we can call it.
        cpp_exe = ROOT / "cpp" / "mpp.exe"
        if not cpp_exe.exists():
            return "cpp\\mpp.exe not found. Build it first (see cpp\\README.md)."
        # Run a small M++ snippet via native runtime (prints output).
        p = subprocess.run([str(cpp_exe), "-e", 'print("Hello from native C++ M++")'], capture_output=True, text=True)
        if p.returncode != 0:
            return f"[cpp mpp error]\n{p.stdout}\n{p.stderr}".strip()
        return p.stdout.strip()


def main():
    # Ensure local imports resolve when running from anywhere.
    os.chdir(str(ROOT))

    api = Api()
    window = webview.create_window(
        "M++ Terminal UI",
        url=str(WEB_DIR / "index.html"),
        js_api=api,
        width=980,
        height=720,
    )
    webview.start(debug=True)


if __name__ == "__main__":
    main()

