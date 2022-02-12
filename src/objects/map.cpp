#include "objects/map.hh"
#include "heap.hh"

namespace runtime {

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

} // namespace runtime