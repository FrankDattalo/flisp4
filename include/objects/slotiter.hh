#ifndef SLOT_ITER_HH__
#define SLOT_ITER_HH__

#include "lib/std.hh"

#include "continuation.hh"
#include "env.hh"
#include "frame.hh"
#include "lambda.hh"
#include "map.hh"
#include "native_function.hh"
#include "object.hh"
#include "pair.hh"
#include "string.hh"
#include "stack.hh"
#include "vector.hh"

class SlotIterator {
private:
    Object* obj;
    std::size_t next_index = 0;
public:
    SlotIterator(Object* _obj)
    : obj{_obj}
    {}

    ~SlotIterator() = default;

    bool HasNext() const {
        struct Visitor : public Object::Visitor {
            bool result = false;
            const SlotIterator* iter;

            Visitor(const SlotIterator* _iter)
            : iter{_iter}
            {}

            #define ADD_CASE(v) void On##v(const v* o) override { \
                result = o->HasNext(iter->next_index); \
            }
            PER_CONCRETE_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE

        } visitor(this);

        obj->Visit(visitor);

        return visitor.result;
    }

    Primitive* Next() {
        struct Visitor : public Object::Visitor {
            Primitive* result = nullptr;
            SlotIterator* iter;

            Visitor(SlotIterator* _iter)
            : iter{_iter}
            {}

            #define ADD_CASE(v) void On##v(const v* o) override { \
                result = o->Next(iter->next_index); \
            }
            PER_CONCRETE_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE

        } visitor(this);

        obj->Visit(visitor);

        next_index += 1;

        return visitor.result;
    }
};

#endif // SLOT_ITER_HH__