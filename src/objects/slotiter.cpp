#include "objects/slotiter.hh"

SlotIterator Object::Slots() {
    return SlotIterator{this};
}
