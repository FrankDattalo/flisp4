#ifndef REFERENCE_HH__
#define REFERENCE_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class Reference : public Primitive {
public:
    Reference(Object* value) {
        SetReference(value);
    }

    ~Reference() = default;

    Object* Value() const {
        return GetReference();
    }
};

#endif // REFERENCE_HH__