#ifndef STRUCTURE_HH__
#define STRUCTURE_HH__

#include "lib/std.hh"
#include "slottedobject.hh"

template<Object::Type TYPE, std::size_t N>
class Structure : public SlottedObject {
public:
    Structure() 
    : SlottedObject(TYPE, AllocationSize()) 
    {}

    constexpr static std::size_t AllocationSize() {
        return sizeof(Object) + sizeof(Primitive) * N;
    }

    constexpr static std::size_t MinAllocationSize() {
        return AllocationSize();
    }

    constexpr static std::size_t NumberOfSlots() {
        return N;
    }
};

#define FIELD(number, name) \
    static_assert(number < NumberOfSlots()); \
    Primitive& name() { return SlotRef(number); } \
    const Primitive& Const##name() const { return *SlotPtr(number); }

#endif // STRUCTURE_HH__