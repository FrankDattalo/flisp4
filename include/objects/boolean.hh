#ifndef BOOLEAN_HH__
#define BOOLEAN_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class Boolean : public Primitive {
public:
    Boolean(bool value) {
        SetBoolean(value);
    }

    ~Boolean() = default;

    NOT_MOVEABLE(Boolean);
    COPYABLE(Boolean);

    bool Value() const {
        return GetBoolean();
    }
};

#endif // BOOLEAN_HH__