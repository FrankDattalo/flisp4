#ifndef INTEGER_HH__
#define INTEGER_HH__

#include "lib.hh"
#include "primitive.hh"

class Integer : public Primitive {
public:
    Integer(std::int64_t value) {
        SetInteger(value);
    }

    ~Integer() = default;

    std::int64_t Value() const {
        return GetInteger();
    }

};

#endif // INTEGER_HH__