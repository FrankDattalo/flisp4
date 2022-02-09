#ifndef VECTOR_HH__
#define VECTOR_HH__

#include "lib/std.hh"
#include "slottedobject.hh"

namespace runtime {

class Vector : public SlottedObject {
public:
    Vector(std::size_t i) 
    : SlottedObject(Object::Type::Vector, AllocationSize(i)) 
    {
        SlotRef(0) = Primitive::Integer(i);
    }

    ~Vector() = default;

    Primitive GetLength() const { return GetSlot(0); }

    Primitive GetItem(std::uint32_t index) const {
        return GetSlot(index + 1);
    }

    void SetItem(std::uint32_t index, Primitive val) {
        SlotRef(index + 1) = val;
    }

    static std::size_t AllocationSize(std::size_t items) {
        return MinAllocationSize() + sizeof(Primitive) * items;
    }

    constexpr static std::size_t MinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }
};

static_assert(sizeof(Vector) == sizeof(Object));

} // namespace runtime

#endif // VECTOR_HH__