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

    Primitive GetFirst() const { return const_first(); }
    void SetFirst(Primitive val) { first() = val; }

    Primitive GetSecond() const { return const_second(); }
    void SetSecond(Primitive val) { second() = val; }
};

static_assert(sizeof(Pair) == sizeof(Object));

} // namespace runtime


#endif // PAIR_HH__