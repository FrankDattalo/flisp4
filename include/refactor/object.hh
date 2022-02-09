#ifndef REFACTOR_OBJECT_HH__
#define REFACTOR_OBJECT_HH__

#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>

#include "util/debug.hh"
#include "util/memory_semantic_macros.hh"

namespace runtime {

static_assert(sizeof(std::int64_t) == 2 * sizeof(std::uint32_t));
static_assert(std::numeric_limits<std::int64_t>::max() >= std::numeric_limits<std::uint32_t>::max());

class Object;

class Primitive {
public:
    #define PER_PRIMITIVE_TYPE(V) \
        V(Nil) \
        V(Reference) \
        V(Integer) \
        V(Symbol) \
        V(Boolean) \
        V(Real) \
        V(Character) \
        V(NativeReference)

    enum class Type {
        #define COMMA(v) v,
        PER_PRIMITIVE_TYPE(COMMA)
        #undef COMMA
    };
private:
    constexpr static std::int64_t TYPE_TAG = 0b111;
    constexpr static std::int64_t DATA_TAG = ~TYPE_TAG;
    constexpr static int TYPE_TAG_SIZE = 3; // in bits
    constexpr static std::int64_t MAX_INT = std::numeric_limits<std::int64_t>::max() >> TYPE_TAG_SIZE;
    constexpr static std::int64_t MIN_INT = std::numeric_limits<std::int64_t>::min() >> TYPE_TAG_SIZE;
    constexpr static std::int64_t MAX_SYMBOL = std::numeric_limits<std::uint64_t>::max() >> TYPE_TAG_SIZE;

    constexpr static std::uint64_t REFERENCE_TAG = 0b000;
    constexpr static std::uint64_t INTEGER_TAG   = 0b001;
    constexpr static std::uint64_t SYMBOL_TAG    = 0b010;
    constexpr static std::uint64_t BOOLEAN_TAG   = 0b011;
    constexpr static std::uint64_t CHAR_TAG      = 0b100;
    constexpr static std::uint64_t REAL_TAG      = 0b101;
    constexpr static std::uint64_t NATIVE_TAG    = 0b110;

    union {
        std::uint64_t _data; // all non float data is here
        struct {
            float _buffer; // this doesnt hold any real data
            float _real;   // all float data is here
        };
    };

    /* data representation
        Nil - represented as a null reference
        Reference - 64 bit pointer, tagged in place with reference tag, accessed by removing tag
        Integer- 61 bit integer, 3 bit tag
        Symbol- 61 bit unsigned integer, 3 bit tag
        Boolean- represented as integer boolean tag
        Real- 32 bit float, 29 bit buffer, 3 bit tag
        Character- represented as integer with boolean tag
        NativeReference - 64 bit pointer, tagged in palce with reference tag, acessed by removing tag
    */
public:
    Primitive() {
        // initialize to nil
        SetNil();
    }

    MOVEABLE(Primitive);
    COPYABLE(Primitive);

    ~Primitive() = default;

    static Primitive Integer(std::int64_t value) {
        Primitive ret;
        ret.SetInteger(value);
        return ret;
    }

    static Primitive Nil() {
        Primitive ret;
        ret.SetNil();
        return ret;
    }

    static Primitive Character(char c) {
        Primitive ret;
        ret.SetCharacter(c);
        return ret;
    }

    void SetInteger(std::int64_t value) {
        checkSize(value);
        std::uint64_t data = *reinterpret_cast<std::uint64_t*>(&value);
        replace(data << TYPE_TAG_SIZE, INTEGER_TAG);
    }

    std::int64_t GetInteger() const {
        checkType(Primitive::Type::Integer);
        return getIntegerData();
    }

    void SetSymbol(std::uint64_t value) {
        checkSize(value);
        replace(value << TYPE_TAG_SIZE, SYMBOL_TAG);
    }

    std::uint64_t GetSymbol() const {
        checkType(Primitive::Type::Symbol);
        std::uint64_t value = data(this->_data) >> TYPE_TAG_SIZE;
        return value;
    }

    void SetBoolean(bool value) {
        SetInteger(value);
        replaceTag(BOOLEAN_TAG);
    }

    bool GetBoolean() const {
        checkType(Primitive::Type::Boolean);
        return getIntegerData();
    }

    void SetCharacter(char value) {
        SetInteger(value);
        replaceTag(CHAR_TAG);
    }

    char GetCharacter() const {
        checkType(Primitive::Type::Character);
        return getIntegerData();
    }

    void SetReal(float real) {
        this->_data = 0;
        this->_real = real;
        replaceTag(REAL_TAG);
    }

    float GetReal() const {
        checkType(Primitive::Type::Real);
        return this->_real;
    }

    void SetNil() {
        SetReference(nullptr);
    }

    void SetReference(Object* ptr) {
        checkAlignment(ptr);
        replace(pointerData(ptr), REFERENCE_TAG);
    }

    Object* GetReference() const {
        checkType(Primitive::Type::Reference);
        return getPointerData(data(this->_data));
    }

    void SetNativeReference(void* ptr) {
        checkAlignment(ptr);
        replace(pointerData(ptr), NATIVE_TAG);
    }

    void* GetNativeReference() const {
        checkType(Primitive::Type::NativeReference);
        return getNativePointerData(data(this->_data));
    }

    Primitive::Type GetType() const {
        return getType();
    }

    std::string static TypeToString(Primitive::Type type) {
        switch (type) {
            #define ADD_CASE(v) case Primitive::Type::v: return #v;
            PER_PRIMITIVE_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in ObjectTypeToString"};
        }
    }

    class Visitor;

    void Visit(Visitor& visitor) const {
        switch (GetType()) {
            #define ADD_CASE(v) case Primitive::Type::v: { visitor.On##v(this); return; }
            PER_PRIMITIVE_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in Visit"};
        }
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        #define ADD_CASE(v) virtual void On##v(const Primitive* obj) = 0;
        PER_PRIMITIVE_TYPE(ADD_CASE)
        #undef ADD_CASE
    };

private:
    static void checkAlignment(void* ptr) {
        std::uint64_t casted = *reinterpret_cast<std::uint64_t*>(&ptr);
        std::uint64_t type_bits = casted & TYPE_TAG;
        if (type_bits != 0) {
            throw std::runtime_error{"Unable to store unaligned pointer"};
        }
    }

    static void checkSize(std::uint64_t val) {
        if (val > MAX_SYMBOL) {
            std::stringstream str;
            str << "Symbol id " << std::to_string(val)
                << " was larger than max value of " << std::to_string(MAX_SYMBOL);
            throw std::runtime_error{str.str()};
        }
    }

    static void checkSize(std::int64_t val) {
        if (val < MIN_INT) {
            std::stringstream str;
            str << "Integer " << std::to_string(val)
                << " was smaller than min value of " << std::to_string(MIN_INT);
            throw std::runtime_error{str.str()};
        }
        if (val > MAX_INT) {
            std::stringstream str;
            str << "Integer " << std::to_string(val)
                << " was larger than max value of " << std::to_string(MAX_INT);
            throw std::runtime_error{str.str()};
        }
    }

    void checkType(Primitive::Type expected) const {
        Primitive::Type actual = getType();
        if (actual != expected) {
            std::stringstream str;
            str << "Incorrect type. "
                << "Wanted: " << TypeToString(expected)
                << " Was: "    << TypeToString(actual);
            throw std::runtime_error{str.str()};
        }
    }

    Primitive::Type getType() const {
        std::uint64_t type_tag = type(this->_data);
        switch (type_tag) {
            case REFERENCE_TAG: {
                Object* ptr = getPointerData(data(this->_data));
                if (ptr == nullptr) {
                    return Primitive::Type::Nil;
                } else {
                    return Primitive::Type::Reference;
                }
            }
            case INTEGER_TAG  : return Primitive::Type::Integer;
            case SYMBOL_TAG   : return Primitive::Type::Symbol;
            case BOOLEAN_TAG  : return Primitive::Type::Boolean;
            case CHAR_TAG     : return Primitive::Type::Character;
            case REAL_TAG     : return Primitive::Type::Real;
            case NATIVE_TAG   : return Primitive::Type::NativeReference;
            default: throw std::runtime_error{"This should never happen in getType"};
        }
    }

    std::int64_t getIntegerData() const {
        std::uint64_t uncasted = data(this->_data);
        std::int64_t unshifted = *reinterpret_cast<std::int64_t*>(&uncasted);
        std::int64_t shifted = unshifted >> TYPE_TAG_SIZE;
        return shifted;
    }

    static std::uint64_t pointerData(void* ptr) { 
        return *reinterpret_cast<std::uint64_t*>(&ptr); 
    }

    static Object* getPointerData(std::uint64_t data) { 
        return *reinterpret_cast<Object**>(&data);
    }

    static void* getNativePointerData(std::uint64_t data) { 
        return *reinterpret_cast<void**>(&data);
    }

    static std::uint64_t data(std::uint64_t value) { return value & DATA_TAG; }
    static std::uint64_t type(std::uint64_t value) { return value & TYPE_TAG; }
    static std::uint64_t join(std::uint64_t _data, std::uint64_t _tag) { return data(_data) | type(_tag); }

    void replace(std::uint64_t value, std::uint64_t tag) {
        this->_data = join(data(value), type(tag));
    }

    void replaceTag(std::uint64_t tag) {
        this->_data = join(data(this->_data), type(tag));
    }
};

static_assert(sizeof(Primitive) == sizeof(std::int64_t));

/*
    Object - base class for all heap
        SlottedObject - represents a heap object that is an array of slots
            Structure - represents a fixed size array of slots
                Pair - basic list pair
                Map - key value lookup
                MapNode - node in map
                Environment - representings the envrionment
                Stack - stack data structure
                Frame - invocation frame
            Vector - scheme vector
            String - string

*/
class Heap;
class SemiSpaceIterator;
class SlotIterator;

#define PER_OBJECT_TYPE(V)\
    V(Pair) \
    V(Vector) \
    V(String) \
    V(Map) \
    V(MapNode) \
    V(Envrionment) \
    V(Stack) \
    V(Frame) \
    V(GcForward)

#define FORWARD_DECLARE(v) class v;
PER_OBJECT_TYPE(FORWARD_DECLARE)
#undef FORWARD_DECLARE

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
            PER_OBJECT_TYPE(ADD_VISITOR)
            #undef ADD_VISITOR
            default: throw std::runtime_error{"Unaccounted object type in Object.Visit"};
        }
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        #define ADD_VISITOR(v) virtual void On##v(const Object*) = 0;
        PER_OBJECT_TYPE(ADD_VISITOR)
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
    PER_OBJECT_TYPE(ADD_CONVERTER)
    #undef ADD_CONVERTER

    std::string static TypeToString(Object::Type type) {
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

class GcForward: public Object {
public:
    GcForward() = delete;

    bool HasNext(std::size_t i) const {
        // this should never be called
        throw std::runtime_error{"GcForward.HasNext should never be called"};
    }
    Primitive* Next(std::size_t i) const {
        // this should never be called
        throw std::runtime_error{"GcForward.Next should never be called"};
    }

    constexpr static std::size_t MinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }
};

static_assert(sizeof(GcForward) == sizeof(Object));

class SlottedObject : public Object {
protected:
    Primitive& SlotRef(std::size_t i) { return *SlotPtr(i); }
    Primitive GetSlot(std::size_t i) const { return *SlotPtr(i); }

    Primitive* SlotPtr(std::size_t i) const {
        if (i >= SlotCount()) {
            throw std::runtime_error{"Out of bounds slot access"};
        }
        Primitive* head = reinterpret_cast<Primitive*>(const_cast<SlottedObject*>(this));
        return &head[i + 1];
    }
public:
    SlottedObject(Object::Type _type, std::uint32_t _allocation_size)
    : Object(_type, _allocation_size)
    {}

    bool HasNext(std::size_t index) const {
        return index < SlotCount();
    }

    Primitive* Next(std::size_t index) const {
        if (!HasNext(index)) {
            throw std::runtime_error{"Next called on SlottedObject without next"};
        }
        return SlotPtr(index);
    }
private:
    std::size_t SlotCount() const {
        return (GetAllocationSize() - sizeof(Object)) / sizeof(Primitive);
    }
};

static_assert(sizeof(SlottedObject) == sizeof(Object));

template<Object::Type TYPE, std::size_t N>
class Structure : public SlottedObject {
public:
    Structure() 
    : SlottedObject(TYPE, AllocationSize()) 
    {
        for (std::size_t i = 0; i < N; i++) {
            SlotRef(i).SetNil();
        }
    }

    constexpr static std::size_t AllocationSize() {
        return sizeof(Object) + sizeof(Primitive) * N;
    }

    constexpr static std::size_t MinAllocationSize() {
        return AllocationSize();
    }
};

#define FIELD(number, name) \
    Primitive& name() { return SlotRef(number); } \
    const Primitive& const_##name() const { return *SlotPtr(number); }

class Pair : public Structure<Object::Type::Pair, 2> {
private:
    FIELD(0, first);
    FIELD(1, second);
public:
    Pair(Primitive _first, Primitive _second) : Structure() {
        first() = _first;
        second() = _second;
    }

    ~Pair() = default;

    Primitive GetFirst() const { return const_first(); }
    Primitive GetSecond() const { return const_second(); }
};

static_assert(sizeof(Pair) == sizeof(Object));

class Vector : public SlottedObject {
private:

public:
    Vector(std::size_t i) 
    : SlottedObject(Object::Type::Vector, AllocationSize(i)) 
    {
        SlotRef(0) = Primitive::Integer(i);
    }

    ~Vector() = default;

    Primitive GetLength() const { return GetSlot(0); }

    Primitive GetItem(std::uint32_t index) const {
        return GetSlot(index + 1);
    }

    void SetItem(std::uint32_t index, Primitive val) {
        SlotRef(index + 1) = val;
    }

    static std::size_t AllocationSize(std::size_t items) {
        return MinAllocationSize() + sizeof(Primitive) * items;
    }

    constexpr static std::size_t MinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }
};

static_assert(sizeof(Vector) == sizeof(Object));

class String : public Object {
public:
    String(const std::string& str)
    : Object(Object::Type::String, AllocationSize(str))
    {
        *length() = Primitive::Integer(str.size());
        char* dest = chars();
        for (std::size_t i = 0; i < str.size(); i++) {
            dest[i] = str.at(i);
        }
    }

    Primitive GetLength() const { return *length(); }

    Primitive GetChar(std::uint32_t index) const {
        if (index >= GetLength().GetInteger()) {
            throw std::runtime_error{"String index out of bounds"};
        }
        char* c = chars();
        return Primitive::Character(c[index]);
    }

    constexpr static std::size_t MinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }

    bool HasNext(std::size_t i) const {
        return false;
    }

    Primitive* Next(std::size_t i) const {
        throw std::runtime_error{"String.Next should never be called"};
    }

    static std::size_t AllocationSize(const std::string& str) {
        DEBUGLN("String size is " << str.size());
        std::size_t string_bytes = str.size() * sizeof(char) + MinAllocationSize();
        DEBUGLN("Unaligned allocation size is " << string_bytes);
        if (string_bytes % sizeof(Object) != 0) {
            // round up to alignment
            DEBUGLN("Rounding up to next alignment before: " << string_bytes);
            string_bytes = (string_bytes / sizeof(Object) + 1) * sizeof(Object);
            DEBUGLN("Rounding up to next alignment after: " << string_bytes);
        }
        return string_bytes;
    }
private:
    Primitive* length() const {
        const Primitive* const_head = reinterpret_cast<const Primitive*>(this);
        Primitive* head = const_cast<Primitive*>(const_head);
        return &head[1];
    }

    char* chars() const {
        const char* data = reinterpret_cast<const char*>(this);
        char* result = const_cast<char*>(data);
        return &result[MinAllocationSize()];
    }
};

static_assert(sizeof(String) == sizeof(Object));

class Map : public Structure<Object::Type::Map, 2> {
private:
    FIELD(0, head);
    FIELD(1, size);
public:
    Map() : Structure() {
        head() = Primitive::Nil();
        size() = Primitive::Integer(0);
    }

    Primitive Lookup(Primitive key) const;
    void Insert(Primitive key, Primitive value);
};

static_assert(sizeof(Map) == sizeof(Object));

class MapNode: public Structure<Object::Type::MapNode, 3> {
private:
    FIELD(0, key);
    FIELD(1, value);
    FIELD(2, next);
public:
    MapNode(Primitive _key, Primitive _value, Primitive _next) : Structure() {
        key() = _key;
        value() = _value;
        next() = _next;
    }
};

static_assert(sizeof(MapNode) == sizeof(Object));

class Envrionment : public Structure<Object::Type::Envrionment, 2> {
private:
    FIELD(0, outer);
    FIELD(1, lookup);

public:
    Envrionment(Primitive _outer, Primitive _lookup) : Structure() {
        outer() = _outer;
        lookup() = _lookup;
    }

    ~Envrionment() = default;

    Map GetLookup() const;
    Primitive GetOuter() const;
};

static_assert(sizeof(Envrionment) == sizeof(Object));

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

    void Push(Primitive obj);
    Primitive Pop();
    Primitive Size() const;
};

static_assert(sizeof(Stack) == sizeof(Object));

class Frame : public Structure<Object::Type::Frame, 3> {
private:
    FIELD(0, env);
    FIELD(1, outer);
    FIELD(2, stack);
public:
    Frame(Primitive _env, Primitive _outer, Primitive _stack) : Structure() {
        env() = _env;
        outer() = _outer;
        stack() = _stack;
    }
};

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

            #define ADD_CASE(v) void On##v(const Object* o) override { \
                result = o->AsConst##v()->HasNext(iter->next_index); \
            }
            PER_OBJECT_TYPE(ADD_CASE)
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

            #define ADD_CASE(v) void On##v(const Object*) override { \
                result = iter->obj->As##v()->Next(iter->next_index); \
            }
            PER_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE

        } visitor(this);

        obj->Visit(visitor);

        next_index += 1;

        return visitor.result;
    }
};

SlotIterator Object::Slots() {
    return SlotIterator{this};
}

// sanity that we can always write a gc forward for any object
#define ASSERT_SIZE(v) static_assert(v::MinAllocationSize() >= Object::RequiredMinAllocationSize());
PER_OBJECT_TYPE(ASSERT_SIZE)
#undef ASSERT_SIZE

}

#endif // REFACTOR_OBJECT_HH__