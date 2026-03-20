"""
Build M++ as a standalone .exe (no Python required to run)
Run: python build_exe.py
Output: dist/mpp.exe
"""
import subprocess
import sys
import os

def main():
    try:
        import PyInstaller
    except ImportError:
        print("Installing PyInstaller...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", "pyinstaller"])
    
    # Ensure we're in the right dir
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    icon_path = os.path.join(script_dir, "assets", "mpp_file_icon.ico")
    
    subprocess.run([
        sys.executable, "-m", "PyInstaller",
        "--onefile",           # Single .exe
        "--name", "mpp",       # Output: mpp.exe
        "--console",           # Console app
        "--icon", icon_path,   # Embed icon into mpp.exe
        "--paths", ".",        # Find mpp package
        "mpp_launch.py",
    ], check=True)
    
    print("\nDone! Run: dist\\mpp.exe -r hello.mpp")
    print("Copy dist\\mpp.exe to a folder in your PATH to use 'mpp' anywhere.")


if __name__ == "__main__":
    main()
