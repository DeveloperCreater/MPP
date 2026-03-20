# M++ — The Ultimate Computer Language

**M++** combines the best of C++, JavaScript, and Python into one powerful, expressive language.

## Design Philosophy

| From C++ | From JavaScript | From Python |
|----------|-----------------|-------------|
| Static typing option | Dynamic flexibility | Clean, readable syntax |
| Explicit control | First-class functions | List comprehensions |
| Performance mindset | Closures & async feel | Indentation & elegance |
| Generics/templates | Object literals | Batteries-included stdlib |

## Syntax Overview

### Variables
```mpp
let x = 42                    // dynamic typing (Python/JS style)
let y: int = 100              // optional static type (C++ style)
const PI = 3.14159            // immutable binding
```

### Functions
```mpp
fn add(a, b) { a + b }        // implicit return, Python elegance

fn factorial(n: int): int {   // typed params & return (C++ style)
    if n <= 1 { 1 }
    else { n * factorial(n - 1) }
}

fn double => x * 2            // arrow function (JS style)
```

### Control Flow
```mpp
if (condition) { ... }        // C++/JS style braces
elif (other) { ... }
else { ... }

for (let i = 0; i < 10; i++) { ... }   // C-style for
for (x in arr) { ... }                  // Python/JS for-in
while (condition) { ... }
```

### Data Structures
```mpp
let arr = [1, 2, 3, 4, 5]     // Arrays (all three)
let obj = { name: "M++", version: 1 }  // Objects (JS style)

// List comprehension (Python power)
let squared = [x * x for x in arr if x > 2]
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
```

### Built-ins
- `print(...)` — output
- `len(x)` — length of array/string
- `range(n)` — 0 to n-1
- `typeof(x)` — type checking

### Low-Level (C++ implementation)
- `mem_alloc(n)` — allocate n bytes, returns ptr
- `mem_read8(ptr, offset)` — read byte at offset
- `mem_write8(ptr, offset, value)` — write byte at offset
