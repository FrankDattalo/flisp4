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
    Primitive Lookup(Primitive key) const {
        Pair* kvpair = LookupNode(key);
        if (kvpair != nullptr) {
            return kvpair->GetSecond();
        }
        return Primitive::Nil();
    }

    void Insert(Heap* heap, Primitive key, Primitive value);

    Primitive Size() const { return const_size(); }

private:
    Pair* LookupNode(Primitive key) const {
        Primitive current = const_head();
        while (current.GetType() != Primitive::Type::Nil) {
            Pair* casted = current.GetReference()->AsPair();
            Pair* kvpair = casted->GetFirst().GetReference()->AsPair();
            current = casted->GetSecond();
            // TODO: make this deep equals
            if (kvpair->GetFirst().ShallowEquals(&key)) {
                return kvpair;
            }
        }
        return nullptr;
    }
};

static_assert(sizeof(Map) == sizeof(Object));

} // namespace runtime

#endif // MAP_HH__