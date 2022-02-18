#include "objects/stack.hh"
#include "heap.hh"

/*
void Stack::Push(Stack* self_, Heap* heap, Primitive obj_) {
    PrimitiveHandle obj = heap->GetHandle(obj_);
    ReferenceHandle<Stack> self = heap->GetHandle(self_);

    Pair* new_head = heap->NewPair(
        obj.GetData(), self->head()
    );

    self->head() = Primitive::Reference(new_head);
    self->size() = Primitive::Integer(self->size().GetInteger() + 1);
}
*/

void Stack::Push(Heap* heap, Handle stack, Handle item) {
    stack.AsStack()->Head() = heap->NewPair(
        item,
        heap->GetHandle(stack.AsStack()->Head())
    );
}


static Handle Pop(Heap* heap, Handle stack) {
    Handle head = heap->GetHandle(stack.AsStack()->Head());
    stack.AsStack()->Head() = head.AsPair()->Second();
    return heap->GetHandle(head.AsPair()->First());
}