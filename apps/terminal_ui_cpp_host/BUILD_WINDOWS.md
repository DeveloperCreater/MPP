## Build & Run `terminal_ui_cpp_host` (Windows)

This app is **native C++** (no Python host). It uses **WebView2** for the HTML/CSS/JS UI and the **C++ M++ runtime** for logic.

### 1) Install prerequisites

- **Visual Studio 2022** with **Desktop development with C++**
- **WebView2 Runtime** (usually already installed on Windows 11)
- **WebView2 SDK headers**:
  - Easiest: in Visual Studio, add NuGet package **`Microsoft.Web.WebView2`**

### 2) Build (Visual Studio)

1. Open Visual Studio
2. **File → Open → Folder…**
3. Select:
   - `c:\Users\xenty\Computer L\M++\apps\terminal_ui_cpp_host`
4. Visual Studio will detect `CMakeLists.txt`
5. Select **x64-Debug** (or Release)
6. Build

### 3) Run

After build, run the generated `TerminalUI.exe` from Visual Studio (Debug → Start Without Debugging).

### Notes

- The UI files live in `web/` and must be next to the exe in the output folder.
- If you want, we can add a post-build step to copy `web/` automatically.

