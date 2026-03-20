## Native C++ Window App (no Python host)

This is the **real** direction you asked for:

- C++ is the **main controller**
- UI is **HTML/CSS/JS**
- M++ runs via the **C++ runtime** (same interpreter code in `M++/cpp/`)
- No Python is required to run the app

### Requirements (Windows)

- Visual Studio 2022 with **Desktop development with C++**
- Microsoft Edge **WebView2 Runtime** (usually already installed on Windows 11)

### Build (Visual Studio / MSVC)

This project uses WebView2 via NuGet. The easiest path is:

1. Open the folder `apps/terminal_ui_cpp_host/` in Visual Studio
2. Build the CMake project

### What’s inside

- `main.cpp`: WebView2 host + message bridge
- `web/`: the UI (HTML/CSS/JS)
- Uses the M++ C++ sources from `M++/cpp/` to evaluate small M++ snippets

### Run

After build, run the produced `TerminalUI.exe`.

