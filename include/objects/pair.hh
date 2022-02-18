#ifndef PAIR_HH__
#define PAIR_HH__

#include "structure.hh"

class Pair : public Structure<Object::Type::Pair, 2> {
public:
    Pair(Handle _first, Handle _second);

    ~Pair() = default;

    FIELD(0, First);

    FIELD(1, Second);
};

static_assert(sizeof(Pair) == sizeof(Object));


#endif // PAIR_HH__