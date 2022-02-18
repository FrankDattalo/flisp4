#include "objects/map.hh"
#include "heap.hh"

/*
void Map::Insert(Map* self_, Heap* heap, Primitive key_, Primitive value_) {

    ReferenceHandle<Map> self = heap->GetHandle(self_);
    PrimitiveHandle key = heap->GetHandle(key_);
    PrimitiveHandle value = heap->GetHandle(value_);

    Pair* lookup = Map::LookupNode(self.GetPointer(), key.GetData());

    // update
    if (lookup != nullptr) {
        Pair::SetSecond(lookup, value.GetData());
        return;
    }

    // insert
    ReferenceHandle<Pair> kvpair = heap->GetHandle(
        heap->NewPair(key.GetData(), value.GetData())
    );

    ReferenceHandle<Pair> new_head = heap->GetHandle(
        heap->NewPair(
            kvpair.GetData(), 
            self->head()
        )
    );

    self->size() = Primitive::Integer(self->size().GetInteger() + 1);
    self->head() = new_head.GetData();
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
*/