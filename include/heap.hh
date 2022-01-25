#ifndef HEAP_HH__
#define HEAP_HH__

#include <stdexcept>
#include <vector>
#include <cstring>

#include "memory_semantic_macros.hh"
#include "object.hh"

class SemiSpace;
class SemiSpaceIterator;

class SemiSpace {
friend SemiSpaceIterator;
private:
    Object* data;
    std::uint64_t data_size;
    std::uint64_t first_free;
public:
    SemiSpace(std::uint64_t size) {
        data = new Object[size];
        data_size = size;
    }

    ~SemiSpace() {
        delete[] data;
    }

    NOT_COPYABLE(SemiSpace);

    NOT_MOVEABLE(SemiSpace);

    bool CanFit(std::uint64_t num_items) {
        return AvailableSlots() >= num_items;
    }

    Object* Allocate(std::uint64_t num_items) {
        if (!CanFit(num_items)) {
            throw std::runtime_error{std::string{"Tried to allocate without enough space"}};
        }
        Object* result = &data[first_free];
        first_free += num_items;
        return result;
    }

    SemiSpaceIterator Slots();

    void Clear() {
        first_free = 0;
    }

    bool Owns(Object* ptr) {
        return &this->data[0] <= ptr && ptr <= &this->data[data_size-1];
    }

private:
    std::uint64_t AvailableSlots() {
        return this->data_size - this->first_free;
    }

    std::uint64_t FirstFree() {
        return this->first_free;
    }

    Object* SlotAtIndex(std::uint64_t index) {
        return &this->data[index];
    }
};

class Heap;

class RootMarker {
public:
    virtual void Mark(Heap* heap) = 0;
};

class Heap {
private:
    SemiSpace space1;
    SemiSpace space2;
    SemiSpace* active;
    SemiSpace* passive;
    std::vector<RootMarker*> markers;
public:
    Heap(std::size_t size) : space1{size}, space2{size} {
        active = &space1;
        passive = &space2;
    }

    ~Heap() = default;

    NOT_COPYABLE(Heap);

    NOT_MOVEABLE(Heap);

    // adds a root marker to the heap
    // any part of the vm that needs to maintain references
    // into the heap needs to add a root marker to mark its roots
    void AddRootMarker(RootMarker* m) {
        this->markers.push_back(m);
    }

    // Allocates an object with items elements and returns a pointer
    // to the object header
    Object* AllocateObject(std::uint64_t items) {
        std::uint64_t total_allocation_elements = items + 1;
        Object* result = Allocate(total_allocation_elements);
        result[0].SetObjectHeader(total_allocation_elements);
        for (std::uint64_t item_index = 0; item_index < items; item_index++) {
            result[item_index + 1].SetNil();
        }
        return result;
    }

private:
    // Primary interface for allocating on the heap
    // the data returned is not initialized and must
    // be initialized before use
    Object* Allocate(std::uint64_t num_items) {

        if (active->CanFit(num_items)) {
            return active->Allocate(num_items);
        }

        Gc();

        if (active->CanFit(num_items)) {
            return active->Allocate(num_items);
        }

        throw std::runtime_error{std::string{"Out of memory"}};
    }
public:

    // Given a slot which should be a reference to a heap allocation
    // transfer it to the new heap. This should be used by RootMarker
    // methods to transfer their references to the new heap
    Object* Transfer(Object* slot) {

        if (slot->IsType(ObjectType::GcForward)) {
            return slot->GetGcForward();
        }

        if (!slot->IsType(ObjectType::GcForward)) {
            throw std::runtime_error{std::string{
                "Expected to transfer a heap object, not a simple value"
            }};
        }
        
        std::uint64_t slots_of_allocation = slot->GetAllocationSize();

        Object* new_spot = active->Allocate(slots_of_allocation);

        memcpy(new_spot, slot, sizeof(Object) * slots_of_allocation);

        slot->SetGcForward(new_spot);

        return new_spot;
    }

    // Given a slot which may or may not live on the heap
    // if the slot is a reference, transfer it to the new heap
    // this is the primary iterface for transferring items to prevent GC
    void TransferIfReference(Object* slot) {
        // if the slot is not a reference
        // then we dont care about it
        if (!slot->IsType(ObjectType::Reference)) {
            return;
        }

        // otherwise, if its pointing to the old heap we must transfer
        // that data to the new heap as it's not garbage
        slot->SetReference(Transfer(slot->GetReference()));
    }

private:
    // Actual GC Implementation here
    void Gc();
};

class SemiSpaceIterator {
private:
    SemiSpace* space;
    std::uint64_t index;
public:
    SemiSpaceIterator(SemiSpace* space) {
        this->space = space;
    }

    bool HasNext() {
        return index < space->FirstFree();
    }

    Object* Next() {
        Object* result = space->SlotAtIndex(index);
        index++;
        return result;
    }
};

// this needs to be here because both semi space and it's iterator need to be defined
// prior to using
SemiSpaceIterator SemiSpace::Slots() {
    return SemiSpaceIterator{this};
}

// this needs to be here because both semi space and it's iterator need to be defined
// prior to using
void Heap::Gc() {
    // swap the spaces 
    SemiSpace* temp = active;
    active = passive;
    passive = temp;

    // marks all the roots, transferring them to the
    // opposite space in the process
    for (auto marker : markers) {
        marker->Mark(this);
    }

    // iterate over all the newly transferred things
    // and pull over all their children
    SemiSpaceIterator iter = active->Slots();
    while (iter.HasNext()) {
        Object* slot = iter.Next();
        TransferIfReference(slot);
    }

    // gc the passive size
    passive->Clear();
}


#endif // HEAP_H__