#ifndef NATIVE_FN_HH__
#define NATIVE_FN_HH__

#include "structure.hh"

namespace runtime {

using NativeFunctionPointer = Primitive(*)();

class NativeFunction : public Structure<Object::Type::NativeFunction, 2> {
private:
    FIELD(0, ptr);
    FIELD(1, arity);
public:
    NativeFunction(Primitive _ptr, Primitive _arity) : Structure() {
        ptr() = _ptr;
        arity() = _arity;
    }

    NativeFunctionPointer Handle() const {
        return reinterpret_cast<NativeFunctionPointer>(const_ptr().GetNativeReference());
    }

    std::int64_t Arity() const {
        return const_arity().GetInteger();
    }
};

};

#endif // NATIVE_FN_HH__