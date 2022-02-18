#ifndef NIL_HH__
#define NIL_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class Nil : public Primitive {
public:
    Nil() {
        SetNil();
    }

    ~Nil() = default;

    NOT_MOVEABLE(Nil);
    COPYABLE(Nil);
};

#endif // REAL_HH__