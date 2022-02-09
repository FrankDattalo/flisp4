#include "refactor/objects/slotiter.hh"

namespace runtime {

SlotIterator Object::Slots() {
    return SlotIterator{this};
}

} // namespace runtime