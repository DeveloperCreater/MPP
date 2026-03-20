# M++ C++ Implementation

Native C++ implementation for **low-level power** and **performance**. No Python or Node.js required.

## Build

### Option 1: MinGW (g++)
```bash
cd cpp
g++ -std=c++17 -O2 -o mpp.exe main.cpp lexer.cpp parser.cpp value.cpp interpreter.cpp
```

### Option 2: Visual Studio (cl)
```cmd
cd cpp
cl /EHsc /std:c++17 /O2 /Fe:mpp.exe main.cpp lexer.cpp parser.cpp value.cpp interpreter.cpp
```

### Option 3: CMake
```bash
cd cpp
cmake -B build
cmake --build build
```

## Run

```bash
mpp.exe -r ..\examples\hello.mpp
mpp.exe ..\examples\hello.mpp
mpp.exe -e "print(42)"
```

## Build apps (packager)

This creates a runnable folder containing `mpp.exe` + your script:

```bash
.\mpp.exe build ..\examples\hello.mpp -o ..\build\hello_app
..\build\hello_app\run.bat
```

## Low-Level Features (C++ Power)

M++ includes raw memory primitives for systems programming:

```mpp
let buf = mem_alloc(64)    // Allocate 64 bytes
mem_write8(buf, 0, 255)    // Write byte at offset 0
let b = mem_read8(buf, 0)  // Read byte
print(b)                   // 255
```

## File Layout

- `main.cpp` - CLI entry point
- `lexer.cpp/hpp` - Tokenizer
- `parser.cpp/hpp` - AST parser
- `ast.hpp` - AST node definitions
- `value.cpp/hpp` - Runtime values (Number, String, Array, Object, RawPtr)
- `interpreter.cpp/hpp` - AST interpreter
