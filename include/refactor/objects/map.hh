#ifndef MAP_HH__
#define MAP_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "pair.hh"

namespace runtime {

class Map : public Structure<Object::Type::Map, 2> {
private:
    FIELD(0, head);
    FIELD(1, size);
public:
    Map() : Structure() {
        head() = Primitive::Nil();
        size() = Primitive::Integer(0);
    }

    Primitive Lookup(Primitive key) const {
        Primitive current = const_head();
        while (current.GetType() != Primitive::Type::Nil) {
            Pair* casted = current.GetReference()->AsPair();
            Pair* kvpair = casted->GetFirst().GetReference()->AsPair();
            current = casted->GetSecond();
            if (kvpair->GetFirst().ShallowEquals(&key)) {
                return kvpair->GetSecond();
            }
        }
        return Primitive::Nil();
    }

    void Insert(Heap* heap, Primitive key, Primitive value);
};

static_assert(sizeof(Map) == sizeof(Object));

} // namespace runtime

#endif // MAP_HH__