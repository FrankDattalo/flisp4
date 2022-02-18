#ifndef REAL_HH__
#define REAL_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class Real : public Primitive {
public:
    Real(float value) {
        SetReal(value);
    }

    ~Real() = default;

    NOT_MOVEABLE(Real);
    COPYABLE(Real);

    float Value() const {
        return GetReal();
    }
};

#endif // REAL_HH__