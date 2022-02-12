#ifndef PAIR_HH__
#define PAIR_HH__

#include "structure.hh"

namespace runtime {

class Pair : public Structure<Object::Type::Pair, 2> {
private:
    FIELD(0, first);
    FIELD(1, second);
public:
    Pair(Primitive _first, Primitive _second) : Structure() {
        first() = _first;
        second() = _second;
    }

    ~Pair() = default;

    static Primitive GetFirst(const Pair* self) { return self->const_first(); }
    static void SetFirst(Pair* self, Primitive val) { self->first() = val; }

    static Primitive GetSecond(const Pair* self) { return self->const_second(); }
    static void SetSecond(Pair* self, Primitive val) { self->second() = val; }
};

static_assert(sizeof(Pair) == sizeof(Object));

} // namespace runtime


#endif // PAIR_HH__