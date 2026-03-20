# M++ — Open Source Programing Language


## Quick Start

**Option A — C++ runtime (primary, recommended):**
```bash
cd M++/cpp
.\build.bat
.\mpp.exe -r ..\examples\hello.mpp
```

**Option B — Python-based .exe (fallback runtime):**
```bash
cd M++
python build_exe.py
dist\mpp.exe -r examples\hello.mpp
```

See `cpp/README.md` for C++ build details.

## Build apps (M++ “compiler”)

M++ apps are packaged using the **native runtime**. The C++ `mpp.exe` supports:

```bash
.\mpp.exe build ..\examples\hello.mpp -o ..\apps\HelloApp
..\apps\HelloApp\run.bat
```

This creates a distributable folder with:
- `mpp.exe` (runtime)
- `app.mpp` (your program)
- `run.bat` (launcher)

### UI apps (HTML/CSS/JS) hosted by M++

M++ UI apps use the app folder format in `apps/APP_FORMAT.md`.

Example app included:
- `apps/editor_app/` (HTML/CSS/JS editor UI + `app.mpp` backend)

Packaging command (C++ runtime):

```bash
.\mpp.exe build-app ..\apps\editor_app -o ..\apps\EditorBuild
```

Window hosting is provided by the native WebView2 host (C++). See `apps/terminal_ui_cpp_host/`.

## Windows installer

See `installer/README.md` to build a real Windows installer that installs `mpp.exe` and can add it to PATH.

## Interop with other languages (run alongside JS/Python/etc.)

M++ has `exec(cmd)` so you can call other languages/tools:

```mpp
print(exec("python -c \"print(6*7)\""))
```

## Tests (release hardening)

Run the regression suite against a runtime:

```bash
# Dist runtime (fallback)
npm run test:dist

# C++ runtime (primary) once you have cpp/mpp.exe built
npm run test:cpp
```

## Features

| From C++ | From JavaScript | From Python |
|----------|-----------------|-------------|
| Optional static typing | Dynamic flexibility | Clean, readable syntax |
| C-style `for` loops | First-class functions | Implicit returns |
| Classes with pub/priv | Arrow functions `=>` | List literals `[1,2,3]` |
| Braces `{}` blocks | Object literals `{a:1}` | Comments `//` |

## Syntax Examples

### Variables & Functions
```mpp
let x = 42
let y: int = 100

fn add(a, b) { a + b }
fn double => x * 2
```

### Control Flow
```mpp
for (let i = 0; i < 10; i = i + 1) {
    print(i)
}

for (x in [1, 2, 3]) {
    print(x)
}

if (x > 0) { print("positive") }
elif (x < 0) { print("negative") }
else { print("zero") }
```

### Classes
```mpp
class Person {
    pub name
    priv age

    fn init(self, name, age) {
        self.name = name
        self.age = age
    }

    fn greet(self) {
        print("Hello, " + self.name)
    }
}

let p = new Person("Alice", 30)
p.greet()
```

### Built-ins
- `print(...)` — output
- `len(x)` — length of array or string
- `range(n)` — `[0, 1, ..., n-1]`
- `typeof(x)` — type string

## Project Structure

```
M++/
├── mpp/
│   ├── lexer.py      # Tokenizer
│   ├── parser.py     # AST builder
│   ├── ast.py        # AST nodes
│   └── interpreter.py# Runtime
├── examples/
│   ├── hello.mpp
│   ├── fibonacci.mpp
│   ├── classes.mpp
│   └── functions.mpp
├── run.py            # Entry point
├── SPEC.md           # Full specification
└── README.md
```

## Requirements

- Python 3.7+
