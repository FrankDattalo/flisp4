#include "objects/stack.hh"
#include "heap.hh"

namespace runtime {

void Stack::Push(Stack* self_, Heap* heap, Primitive obj_) {
    PrimitiveHandle obj = heap->GetHandle(obj_);
    ReferenceHandle<Stack> self = heap->GetHandle(self_);

    Pair* new_head = heap->NewPair(
        obj.GetData(), self->head()
    );

    self->head() = Primitive::Reference(new_head);
    self->size() = Primitive::Integer(self->size().GetInteger() + 1);
}

}