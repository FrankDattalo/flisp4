#ifndef VECTOR_HH__
#define VECTOR_HH__

#include "lib/std.hh"
#include "slottedobject.hh"

class Vector : public SlottedObject {
public:
    Vector(std::size_t i);

    ~Vector() = default;

    Integer Length() const { return *SlotPtr(0)->AsConstInteger(); }

    Primitive GetItem(Integer index) const {
        return GetSlot(index.Value() + 1);
    }

    void SetItem(Integer index, Primitive val) {
        SlotRef(index.Value() + 1) = val;
    }

    static std::size_t AllocationSize(std::size_t items) {
        return MinAllocationSize() + sizeof(Primitive) * items;
    }

    constexpr static std::size_t MinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }
};

static_assert(sizeof(Vector) == sizeof(Object));

#endif // VECTOR_HH__