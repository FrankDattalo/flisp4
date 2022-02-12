#ifndef STACK_HH__
#define STACK_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "primitive.hh"
#include "pair.hh"

namespace runtime {

class Stack : public Structure<Object::Type::Stack, 2> {
private:
    FIELD(0, head);
    FIELD(1, size);

public:
    Stack() : Structure() {
        head() = Primitive::Nil();
        size() = Primitive::Integer(0);
    }

    ~Stack() = default;

    static void Push(Stack* self, Heap* heap, Primitive obj);

    static Primitive Pop(Stack* self) {
        Primitive top = self->head();
        if (top.GetType() == Primitive::Type::Nil) {
            throw std::runtime_error{"Pop on empty stack"};
        }
        Pair* pair = top.GetReference()->AsPair();
        Primitive result = Pair::GetFirst(pair);
        self->head() = Pair::GetSecond(pair);
        self->size() = Primitive::Integer(self->size().GetInteger() - 1);
        return result;
    }

    static Primitive Size(Stack* self) {
        return self->const_size();
    }
};

static_assert(sizeof(Stack) == sizeof(Object));


} // namespace runtime

#endif // STACK_HH__