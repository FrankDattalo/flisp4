#ifndef NATIVE_REFERENCE_HH__
#define NATIVE_REFERENCE_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class NativeReference : public Primitive {
public:
    NativeReference(void* value) {
        SetNativeReference(value);
    }

    ~NativeReference() = default;

    NOT_MOVEABLE(NativeReference);
    COPYABLE(NativeReference);

    void* Value() const {
        return GetNativeReference();
    }
};

#endif // NATIVE_REFERENCE_HH__