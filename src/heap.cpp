#include "heap.hh"

namespace runtime {

SemiSpaceIterator SemiSpace::Iterator() {
    return SemiSpaceIterator{this};
}

Handle HandleManager::Get() {
    Handle ret{this};
    return ret;
}

void Heap::transfer() {
    SemiSpaceIterator iter = active->Iterator();
    while (iter.HasNext()) {
        Object* obj = iter.Next();
        SlotIterator slots = obj->Slots();
        while (slots.HasNext()) {
            Primitive* slot = slots.Next();
            transferIfReference(slot);
        }
    }
}

} // namespace runtime