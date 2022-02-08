#ifndef REFACTOR_OBJECT_HH__
#define REFACTOR_OBJECT_HH__

#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>

#include "util/debug.hh"

namespace runtime {

static_assert(sizeof(std::int64_t) == 2 * sizeof(std::uint32_t));
static_assert(std::numeric_limits<std::int64_t>::max() >= std::numeric_limits<std::uint32_t>::max());

class Object;

class Primitive {
public:
    #define PER_OBJECT_TYPE(V) \
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
        PER_OBJECT_TYPE(COMMA)
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
        SetReference(nullptr);
    }

    ~Primitive() = default;

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
            PER_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in ObjectTypeToString"};
        }
    }

    class Visitor;

    void Visit(Visitor& visitor) const {
        switch (GetType()) {
            #define ADD_CASE(v) case Primitive::Type::v: { visitor.On##v(this); return; }
            PER_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in Visit"};
        }
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        #define ADD_CASE(v) virtual void On##v(const Primitive* obj) = 0;
        PER_OBJECT_TYPE(ADD_CASE)
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
                << "Was: "    << TypeToString(actual);
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
class Object {
public:
    enum class Type {
        Pair,
        Vector,
        String,
        Map,
        MapNode,
        Envrionment,
        Stack,
        Frame
    };
private:
    std::uint32_t type;
    std::uint32_t allocation_size;
protected:
    Type GetType() const;
    std::uint32_t GetAllocationSize() const;
};

static_assert(sizeof(Object) == sizeof(std::int64_t));
static_assert(sizeof(Object) == sizeof(Object));
static_assert(sizeof(Object) % 8 == 0);

class SlottedObject : public Object {
protected:
    Primitive& SlotRef(std::size_t i);
    Primitive GetSlot(std::size_t i) const;
};

static_assert(sizeof(SlottedObject) == sizeof(Object));

template<std::size_t N>
class Structure : public SlottedObject {
};

#define FIELD(number, name) Primitive& name() { return SlotRef(number); }

class Pair : public Structure<2> {
private:
    FIELD(0, first);
    FIELD(1, second);
};

static_assert(sizeof(Pair) == sizeof(Object));

class Vector : public SlottedObject {
public:
    Primitive GetLength() const { return GetSlot(0); }
    Primitive GetItem(std::uint32_t index) const { return GetSlot(index + 1); }
};

static_assert(sizeof(Vector) == sizeof(Object));

class String : public SlottedObject {
private:
    Primitive GetLength() const { return GetSlot(0); }
    Primitive GetChar(std::uint32_t index) const { return GetSlot(index + 1); }
};

static_assert(sizeof(String) == sizeof(Object));

class Map : public Structure<2> {
private:
    FIELD(0, head);
    FIELD(1, size);
public:
    Primitive Lookup(Primitive key) const;
    void Insert(Primitive key, Primitive value);
};

static_assert(sizeof(Map) == sizeof(Object));

class Envrionment : public Structure<2> {
private:
    FIELD(0, outer);
    FIELD(1, lookup);

public:
    Map GetLookup() const;
    Primitive GetOuter() const;
};

static_assert(sizeof(Envrionment) == sizeof(Object));

class Stack : public Structure<2> {
private:
    FIELD(0, head);
    FIELD(1, size);

public:
    void Push(Primitive obj);
    Primitive Pop();
    Primitive Size() const;
};

static_assert(sizeof(Stack) == sizeof(Object));

class Frame : public Structure<3> {
private:
    FIELD(0, env);
    FIELD(1, outer);
    FIELD(2, stack);
};

}

#endif // REFACTOR_OBJECT_HH__