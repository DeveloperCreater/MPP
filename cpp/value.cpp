#include "value.hpp"
#include <sstream>
#include <cmath>

namespace mpp {

RawMem::RawMem(size_t n) : size(n) {
    data = new uint8_t[n];
    for (size_t i = 0; i < n; i++) data[i] = 0;
}

RawMem::~RawMem() {
    delete[] data;
    data = nullptr;
}

std::string Value::toString() const {
    switch (type) {
        case Type::Null: return "null";
        case Type::Number: {
            if (std::floor(numVal) == numVal)
                return std::to_string(static_cast<int64_t>(numVal));
            return std::to_string(numVal);
        }
        case Type::String: return strVal;
        case Type::Bool: return boolVal ? "true" : "false";
        case Type::Array: return arrVal ? "[array]" : "[array]";
        case Type::Object: return objVal ? "[object]" : "[object]";
        case Type::UserFunc: return "[function]";
        case Type::Native: return "[native]";
        case Type::RawPtr: return "[ptr]";
    }
    return "null";
}

bool Value::isTruthy() const {
    switch (type) {
        case Type::Null: return false;
        case Type::Bool: return boolVal;
        case Type::Number: return numVal != 0;
        case Type::String: return !strVal.empty();
        default: return true;
    }
}

bool Value::equals(const Value& other) const {
    if (type != other.type) {
        if (type == Type::Number && other.type == Type::Number)
            return numVal == other.numVal;
        return false;
    }
    switch (type) {
        case Type::Null: return true;
        case Type::Number: return numVal == other.numVal;
        case Type::String: return strVal == other.strVal;
        case Type::Bool: return boolVal == other.boolVal;
        default: return false;
    }
}

} // namespace mpp
