#include "refactor/objects/stack.hh"
#include "refactor/heap.hh"

namespace runtime {

void Stack::Push(Heap* heap, Primitive obj) {
    Handle obj_handle = heap->GetHandle();
    Handle this_handle = heap->GetHandle();

    obj_handle = obj;
    this_handle = Primitive::Reference(this);

    this->head() = Primitive::Reference(heap->NewPair(obj_handle.GetData(), this->head()));
    this->size() = Primitive::Integer(this->size().GetInteger() + 1);
}

}