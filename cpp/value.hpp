#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>

namespace mpp {

struct Interpreter;
struct ASTNode;

using Number = double;
using String = std::string;
using NativeFn = std::function<void(Interpreter&, const std::vector<struct Value>&)>;

// User-defined function (params + body + closure)
struct UserFunc {
    std::vector<std::string> params;
    ASTNode* body = nullptr;  // Non-owning, points into AST
    std::map<std::string, struct Value> closure;
};

// Raw memory block for low-level access (C++ power)
struct RawMem {
    uint8_t* data;
    size_t size;
    RawMem(size_t n);
    ~RawMem();
    RawMem(const RawMem&) = delete;
    RawMem& operator=(const RawMem&) = delete;
};

struct Value {
    enum class Type { Null, Number, String, Bool, Array, Object, UserFunc, Native, RawPtr };
    Type type = Type::Null;
    
    Number numVal = 0;
    String strVal;
    bool boolVal = false;
    std::shared_ptr<std::vector<Value>> arrVal;
    std::shared_ptr<std::map<std::string, Value>> objVal;
    std::shared_ptr<UserFunc> userFunc;
    NativeFn nativeVal;
    std::shared_ptr<RawMem> rawPtr;  // Low-level: raw memory handle

    Value() = default;
    Value(Number n) : type(Type::Number), numVal(n) {}
    Value(const String& s) : type(Type::String), strVal(s) {}
    Value(bool b) : type(Type::Bool), boolVal(b) {}
    Value(NativeFn f) : type(Type::Native), nativeVal(std::move(f)) {}
    
    std::string toString() const;
    bool isTruthy() const;
    bool equals(const Value& other) const;
};

} // namespace mpp
