#include "objects/stack.hh"
#include "heap.hh"

namespace runtime {

void Stack::Push(Heap* heap, Primitive obj) {
    Handle obj_handle = heap->GetHandle(obj);
    Handle this_handle = heap->GetHandle(Primitive::Reference(this));

    Pair* new_head = heap->NewPair(
        obj_handle.GetData(), this->head()
    );

    this->head() = Primitive::Reference(new_head);
    this->size() = Primitive::Integer(this->size().GetInteger() + 1);
}

}