#include "objects/stack.hh"
#include "heap.hh"

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