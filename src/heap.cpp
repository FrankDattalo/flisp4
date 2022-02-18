#include "heap.hh"

SemiSpaceIterator SemiSpace::Iterator() {
    return SemiSpaceIterator{this};
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