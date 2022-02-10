#include "objects/map.hh"
#include "heap.hh"

namespace runtime {

void Map::Insert(Heap* heap, Primitive key, Primitive value) {
    Pair* kvpair = this->LookupNode(key);

    // update
    if (kvpair != nullptr) {
        kvpair->SetSecond(value);
        return;
    }

    // insert
    Handle this_handle = heap->GetHandle(Primitive::Reference(this));
    Handle key_handle = heap->GetHandle(key);
    Handle value_handle = heap->GetHandle(value);
    Handle kvpair_handle = heap->GetHandle(
        Primitive::Reference(
            heap->NewPair(key_handle.GetData(), value_handle.GetData())
        )
    );
    Handle newhead_handle = heap->GetHandle(
        Primitive::Reference(
            heap->NewPair(kvpair_handle.GetData(), head())
        )
    );

    size() = Primitive::Integer(size().GetInteger() + 1);
    head() = newhead_handle.GetData();
}

} // namespace runtime