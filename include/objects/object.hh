#ifndef OBJECT_HH__
#define OBJECT_HH__

#include "lib/std.hh"
#include "primitive.hh"
#include "reference.hh"
#include "util/memory_semantic_macros.hh"

class Heap;
class Handle;
class SemiSpaceIterator;
class SlotIterator;

#define PER_OBJECT_TYPE(V) \
    PER_CONCRETE_OBJECT_TYPE(V) \
    V(GcForward)

#define PER_CONCRETE_OBJECT_TYPE(V) \
    V(Pair) \
    V(Vector) \
    V(String) \
    V(Map) \
    V(Stack) \
    V(Envrionment) \
    V(Frame) \
    V(NativeFunction) \
    V(Lambda) \
    V(Continuation)

#define FORWARD_DECLARE(v) class v;
PER_CONCRETE_OBJECT_TYPE(FORWARD_DECLARE)
#undef FORWARD_DECLARE

/*
    Object - base class for all things allocated on the heap
        SlottedObject - represents a heap object that is an array of slots
            Structure - represents a fixed size array of slots
                Pair - basic list pair
                Map - key value lookup
                Environment - representings the envrionment
                Frame - invocation frame
                NativeFunction - object that holds metadata and pointer to native function 
                Lambda - closure of function including created envrionment
                Continuation - a continuation of a previous stack frame
            Vector - scheme vector created with a variable size of elements
        String - string
*/


class Object {
friend Heap;
friend SemiSpaceIterator;
public:
    enum class Type {
        #define COMMA(v) v,
        PER_OBJECT_TYPE(COMMA)
        #undef COMMA
    };
private:
    Object::Type type;
    std::uint32_t allocation_size;
protected:
    Type GetType() const {
        return type;
    }
public:
    Object(Object::Type _type, std::uint32_t _allocation_size) 
    : type{_type}, allocation_size{_allocation_size} 
    {}

    ~Object() = default;
    NOT_MOVEABLE(Object);
    COPYABLE(Object);

    class Visitor;

    void Visit(Visitor& visitor) const {
        switch (GetType()) {
            #define ADD_VISITOR(v) case Object::Type::v: { visitor.On##v(this->AsConst##v()); return; }
            PER_CONCRETE_OBJECT_TYPE(ADD_VISITOR)
            #undef ADD_VISITOR
            default: throw std::runtime_error{"Unaccounted object type in Object.Visit"};
        }
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        #define ADD_VISITOR(v) virtual void On##v(const v*) = 0;
        PER_CONCRETE_OBJECT_TYPE(ADD_VISITOR)
        #undef ADD_VISITOR
    };

    #define ADD_CONVERTER(v)\
        const v* AsConst##v() const { \
            checkType(Object::Type::v); \
            return reinterpret_cast<const v*>(this); \
        } \
        v* As##v() { \
            checkType(Object::Type::v); \
            return reinterpret_cast<v*>(this); \
        }
    PER_CONCRETE_OBJECT_TYPE(ADD_CONVERTER)
    #undef ADD_CONVERTER

    static std::string TypeToString(Object::Type type) {
        switch (type) {
            #define ADD_CASE(v) case Object::Type::v: return #v;
            PER_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in TypeToString"};
        }
    }

    constexpr static std::size_t RequiredMinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }

protected:
    std::size_t GetAllocationSize() const { return allocation_size; }

private:
    void checkType(Object::Type expected) const {
        Object::Type actual = GetType();
        if (actual != expected) {
            std::stringstream str;
            str << "Incorrect type. "
                << "Wanted: " << TypeToString(expected)
                << " Was: "    << TypeToString(actual);
            throw std::runtime_error{str.str()};
        }
    }

    bool IsGcForward() const { return Type() == Object::Type::GcForward; }

    void SetGcForwardAddress(Object* addr) {
        if (this->allocation_size < sizeof(Object) + sizeof(Primitive)) {
            throw std::runtime_error{"Could not set gc forward on object, too small"};
        }
        Primitive* head = reinterpret_cast<Primitive*>(this);
        this->type = Object::Type::GcForward;
        head[1] = Reference(addr);
    }

    Object* GetGcForwardAddress() const {
        if (!IsGcForward()) {
            throw std::runtime_error{"Not a gc forward object"};
        }
        Primitive* head = reinterpret_cast<Primitive*>(const_cast<Object*>(this));
        return head[1].AsConstReference()->Value();
    }

    SlotIterator Slots();
};

static_assert(sizeof(Object) == sizeof(std::int64_t));
static_assert(sizeof(Object) == sizeof(Primitive));
static_assert(sizeof(Object) % 8 == 0);

#endif // OBJECT_HH__