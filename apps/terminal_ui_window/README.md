## M++ Terminal UI Window App

This is a simple desktop app that opens a **real window** with a **HTML/CSS/JavaScript** interface.
Buttons call into an M++ backend so you can test:

- **M++ logic**
- **JavaScript UI**
- **CSS styling**
- Optional **C++ runtime** execution (if you built `cpp\mpp.exe`)

### Run (dev)

```powershell
cd "c:\Users\xenty\Computer L\M++"
python -m pip install pywebview
python apps\terminal_ui_window\main.py
```

### What it does

- The UI is in `web/index.html`, `web/style.css`, `web/app.js`
- The backend is `main.py`
- The backend runs M++ from `examples/terminal_ui.mpp` as a **library of functions**

