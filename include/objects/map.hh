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

    // TODO make this RB tree
    static Primitive Lookup(const Map* self, Primitive key) {
        Pair* kvpair = LookupNode(self, key);
        if (kvpair != nullptr) {
            return Pair::GetSecond(kvpair);
        }
        return Primitive::Nil();
    }

    static void Insert(Map* self, Heap* heap, Primitive key, Primitive value);

    static Primitive Size(const Map* self) { return self->const_size(); }

private:
    static Pair* LookupNode(const Map* self, Primitive key) {
        Primitive current = self->const_head();
        while (current.GetType() != Primitive::Type::Nil) {
            Pair* casted = current.GetReference()->AsPair();
            Pair* kvpair = Pair::GetFirst(casted).GetReference()->AsPair();
            current = Pair::GetSecond(casted);
            // TODO: make this deep equals
            if (Pair::GetFirst(kvpair).ShallowEquals(&key)) {
                return kvpair;
            }
        }
        return nullptr;
    }
};

static_assert(sizeof(Map) == sizeof(Object));

} // namespace runtime

#endif // MAP_HH__