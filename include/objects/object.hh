#ifndef OBJECT_HH__
#define OBJECT_HH__

#include "lib/std.hh"
#include "primitive.hh"
#include "util/memory_semantic_macros.hh"

namespace runtime {

class Heap;
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
    V(Envrionment) \
    V(Stack) \
    V(Frame) \
    V(VirtualMachine) \
    V(NativeFunction) \
    V(Function) \
    V(Closure)

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
                Stack - stack data structure
                Frame - invocation frame
                NativeFunction - object that holds metadata and pointer to native function 
                Function - holds static function data such as code and formals
                Closure - closure of function including created envrionment
                VirtualMachine - the entire virtual machine
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
            #define ADD_VISITOR(v) case Object::Type::v: { visitor.On##v(this); return; }
            PER_CONCRETE_OBJECT_TYPE(ADD_VISITOR)
            #undef ADD_VISITOR
            default: throw std::runtime_error{"Unaccounted object type in Object.Visit"};
        }
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        #define ADD_VISITOR(v) virtual void On##v(const Object*) = 0;
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
        head[1].SetReference(addr);
    }

    Object* GetGcForwardAddress() const {
        if (!IsGcForward()) {
            throw std::runtime_error{"Not a gc forward object"};
        }
        Primitive* head = reinterpret_cast<Primitive*>(const_cast<Object*>(this));
        return head[1].GetReference();
    }

    SlotIterator Slots();
};

static_assert(sizeof(Object) == sizeof(std::int64_t));
static_assert(sizeof(Object) == sizeof(Primitive));
static_assert(sizeof(Object) % 8 == 0);

}

#endif // OBJECT_HH__