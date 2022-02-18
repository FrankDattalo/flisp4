#ifndef NATIVE_FN_HH__
#define NATIVE_FN_HH__

#include "structure.hh"

using NativeFunctionPointer = void (*)();

class NativeFunction : public Structure<Object::Type::NativeFunction, 2> {
public:
    NativeFunction(Handle _ptr, Handle _arity);

    FIELD(0, Pointer);

    FIELD(1, Arity);
};

#endif // NATIVE_FN_HH__