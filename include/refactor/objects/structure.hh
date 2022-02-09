#ifndef STRUCTURE_HH__
#define STRUCTURE_HH__

#include "lib/std.hh"
#include "slottedobject.hh"

namespace runtime {

template<Object::Type TYPE, std::size_t N>
class Structure : public SlottedObject {
public:
    Structure() 
    : SlottedObject(TYPE, AllocationSize()) 
    {
        for (std::size_t i = 0; i < N; i++) {
            SlotRef(i).SetNil();
        }
    }

    constexpr static std::size_t AllocationSize() {
        return sizeof(Object) + sizeof(Primitive) * N;
    }

    constexpr static std::size_t MinAllocationSize() {
        return AllocationSize();
    }
};

#define FIELD(number, name) \
    Primitive& name() { return SlotRef(number); } \
    const Primitive& const_##name() const { return *SlotPtr(number); }

} // namespace runtime

#endif // STRUCTURE_HH__