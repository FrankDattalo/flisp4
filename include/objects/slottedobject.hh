#ifndef SLOTTED_OBJECT_HH__
#define SLOTTED_OBJECT_HH__

#include "lib/std.hh"
#include "primitive.hh"
#include "object.hh"
#include "nil.hh"

class SlottedObject : public Object {
protected:
    Primitive& SlotRef(std::size_t i) { return *SlotPtr(i); }
    Primitive GetSlot(std::size_t i) const { return *SlotPtr(i); }

    Primitive* SlotPtr(std::size_t i) const {
        if (i >= SlotCount()) {
            throw std::runtime_error{"Out of bounds slot access"};
        }
        Primitive* head = reinterpret_cast<Primitive*>(const_cast<SlottedObject*>(this));
        return &head[i + 1];
    }
public:
    SlottedObject(Object::Type _type, std::uint32_t _allocation_size)
    : Object(_type, _allocation_size)
    {
        std::size_t N = SlotCount();
        for (std::size_t i = 0; i < N; i++) {
            SlotRef(i) = Nil();
        }
    }

    bool HasNext(std::size_t index) const {
        return index < SlotCount();
    }

    Primitive* Next(std::size_t index) const {
        if (!HasNext(index)) {
            throw std::runtime_error{"Next called on SlottedObject without next"};
        }
        return SlotPtr(index);
    }
private:
    std::size_t SlotCount() const {
        return (GetAllocationSize() - sizeof(Object)) / sizeof(Primitive);
    }
};

static_assert(sizeof(SlottedObject) == sizeof(Object));

#endif // SLOTTED_OBJECT_HH__