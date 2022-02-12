#include "objects/map.hh"
#include "heap.hh"

namespace runtime {

void Map::Insert(Map* self, Heap* heap, Primitive key, Primitive value) {
    Pair* kvpair = Map::LookupNode(self, key);

    // update
    if (kvpair != nullptr) {
        Pair::SetSecond(kvpair, value);
        return;
    }

    // insert
    Handle this_handle = heap->GetHandle(Primitive::Reference(self));
    Handle key_handle = heap->GetHandle(key);
    Handle value_handle = heap->GetHandle(value);

    TypedHandle<Map> self_handle = heap->GetHandle(self);

    self_handle->

    Handle kvpair_handle = heap->GetHandle(
        Primitive::Reference(
            heap->NewPair(key_handle.GetData(), value_handle.GetData())
        )
    );
    Handle newhead_handle = heap->GetHandle(
        Primitive::Reference(
            heap->NewPair(kvpair_handle.GetData(), this_handle->GetReference()->AsMap()->head())
        )
    );

    this_handle->GetReference()->AsMap()->size() = Primitive::Integer(
        this_handle->GetReference()->AsMap()->size().GetInteger() + 1);
    this_handle->GetReference()->AsMap()->head() = newhead_handle.GetData();
}

} // namespace runtime