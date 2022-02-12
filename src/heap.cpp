#include "heap.hh"

namespace runtime {

UntypedHandle::UntypedHandle(RootManager* _manager, Primitive _location)
{
    this->manager = _manager;
    this->location = _location;
    this->manager->AddRoot(&location);
}

UntypedHandle::~UntypedHandle() {
    this->manager->RemoveRoot(&location);
}

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

} // namespace runtime