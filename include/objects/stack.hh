#ifndef STACK_HH__
#define STACK_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "pair.hh"

class Stack : public Structure<Object::Type::Stack, 1> {
public:
    Stack();

    ~Stack() = default;

    FIELD(0, Head);

    static void Push(Heap* heap, Handle stack, Handle data);

    static Handle Pop(Heap* heap, Handle stack);
};

static_assert(sizeof(Stack) == sizeof(Object));

#endif // MAP_HH__