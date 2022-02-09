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

    void Push(Heap* heap, Primitive obj);

    Primitive Pop() {
        Primitive top = head();
        if (top.GetType() == Primitive::Type::Nil) {
            throw std::runtime_error{"Pop on empty stack"};
        }
        Pair* pair = top.GetReference()->AsPair();
        Primitive result = pair->GetFirst();
        head() = pair->GetSecond();
        size() = Primitive::Integer(size().GetInteger() - 1);
        return result;
    }

    Primitive Size() const {
        return const_size();
    }
};

static_assert(sizeof(Stack) == sizeof(Object));


} // namespace runtime

#endif // STACK_HH__