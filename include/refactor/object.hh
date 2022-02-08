#ifndef REFACTOR_OBJECT_HH__
#define REFACTOR_OBJECT_HH__

#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>

#include "util/debug.hh"

namespace runtime {

typedef std::uint64_t symbol_id_t;
typedef std::int64_t integer_t;
typedef std::uint32_t uinteger_t;
typedef char char_t;
typedef float real_t;

static_assert(sizeof(integer_t) == 2 * sizeof(uinteger_t));
static_assert(std::numeric_limits<integer_t>::max() >= std::numeric_limits<uinteger_t>::max());

class HeapObject;

class Object {
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
    constexpr static integer_t TYPE_TAG = 0b111;
    constexpr static integer_t DATA_TAG = ~TYPE_TAG;
    constexpr static int TYPE_TAG_SIZE = 3; // in bits
    constexpr static integer_t MAX_INT = std::numeric_limits<integer_t>::max() >> TYPE_TAG_SIZE;
    constexpr static integer_t MIN_INT = std::numeric_limits<integer_t>::min() >> TYPE_TAG_SIZE;
    constexpr static integer_t MAX_SYMBOL = std::numeric_limits<symbol_id_t>::max() >> TYPE_TAG_SIZE;

    constexpr static symbol_id_t REFERENCE_TAG = 0b000;
    constexpr static symbol_id_t INTEGER_TAG   = 0b001;
    constexpr static symbol_id_t SYMBOL_TAG    = 0b010;
    constexpr static symbol_id_t BOOLEAN_TAG   = 0b011;
    constexpr static symbol_id_t CHAR_TAG      = 0b100;
    constexpr static symbol_id_t REAL_TAG      = 0b101;
    constexpr static symbol_id_t NATIVE_TAG    = 0b110;

    union {
        symbol_id_t _data;
        struct {
            real_t _buffer;
            real_t _real;
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
    Object() {
        SetReference(nullptr);
    }

    ~Object() = default;

    void SetInteger(integer_t value) {
        checkSize(value);
        symbol_id_t data = *reinterpret_cast<symbol_id_t*>(&value);
        replace(data << TYPE_TAG_SIZE, INTEGER_TAG);
    }

    integer_t GetInteger() const {
        checkType(Object::Type::Integer);
        return getIntegerData();
    }

    void SetSymbol(symbol_id_t value) {
        checkSize(value);
        replace(value << TYPE_TAG_SIZE, SYMBOL_TAG);
    }

    symbol_id_t GetSymbol() const {
        checkType(Object::Type::Symbol);
        symbol_id_t value = data(this->_data) >> TYPE_TAG_SIZE;
        return value;
    }

    void SetBoolean(bool value) {
        SetInteger(value);
        replaceTag(BOOLEAN_TAG);
    }

    bool GetBoolean() const {
        checkType(Object::Type::Boolean);
        return getIntegerData();
    }

    void SetCharacter(char_t value) {
        SetInteger(value);
        replaceTag(CHAR_TAG);
    }

    char_t GetCharacter() const {
        checkType(Object::Type::Character);
        return getIntegerData();
    }

    void SetReal(real_t real) {
        this->_data = 0;
        this->_real = real;
        replaceTag(REAL_TAG);
    }

    real_t GetReal() const {
        checkType(Object::Type::Real);
        return this->_real;
    }

    void SetReference(HeapObject* ptr) {
        checkAlignment(ptr);
        replace(pointerData(ptr), REFERENCE_TAG);
    }

    HeapObject* GetReference() const {
        checkType(Object::Type::Reference);
        return getPointerData(data(this->_data));
    }

    void SetNativeReference(void* ptr) {
        checkAlignment(ptr);
        replace(pointerData(ptr), NATIVE_TAG);
    }

    void* GetNativeReference() const {
        checkType(Object::Type::NativeReference);
        return getNativePointerData(data(this->_data));
    }

    Object::Type GetType() const {
        return getType();
    }

    std::string static ObjectTypeToString(Object::Type type) {
        switch (type) {
            #define ADD_CASE(v) case Object::Type::v: return #v;
            PER_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in ObjectTypeToString"};
        }
    }

    class Visitor;

    void Visit(Visitor& visitor) const {
        switch (GetType()) {
            #define ADD_CASE(v) case Object::Type::v: { visitor.On##v(this); return; }
            PER_OBJECT_TYPE(ADD_CASE)
            #undef ADD_CASE
            default: throw std::runtime_error{"This should never happen in Visit"};
        }
    }

    class Visitor {
    public:
        virtual ~Visitor() = default;
        #define ADD_CASE(v) virtual void On##v(const Object* obj) = 0;
        PER_OBJECT_TYPE(ADD_CASE)
        #undef ADD_CASE
    };

private:
    static void checkAlignment(void* ptr) {
        symbol_id_t casted = *reinterpret_cast<symbol_id_t*>(&ptr);
        symbol_id_t type_bits = casted & TYPE_TAG;
        if (type_bits != 0) {
            throw std::runtime_error{"Unable to store unaligned pointer"};
        }
    }

    static void checkSize(symbol_id_t val) {
        if (val > MAX_SYMBOL) {
            std::stringstream str;
            str << "Symbol id " << std::to_string(val)
                << " was larger than max value of " << std::to_string(MAX_SYMBOL);
            throw std::runtime_error{str.str()};
        }
    }

    static void checkSize(integer_t val) {
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

    void checkType(Object::Type expected) const {
        Object::Type actual = getType();
        if (actual != expected) {
            std::stringstream str;
            str << "Incorrect type. "
                << "Wanted: " << ObjectTypeToString(expected)
                << "Was: "    << ObjectTypeToString(actual);
            throw std::runtime_error{str.str()};
        }
    }

    Object::Type getType() const {
        symbol_id_t type_tag = type(this->_data);
        switch (type_tag) {
            case REFERENCE_TAG: {
                HeapObject* ptr = getPointerData(data(this->_data));
                if (ptr == nullptr) {
                    return Object::Type::Nil;
                } else {
                    return Object::Type::Reference;
                }
            }
            case INTEGER_TAG  : return Object::Type::Integer;
            case SYMBOL_TAG   : return Object::Type::Symbol;
            case BOOLEAN_TAG  : return Object::Type::Boolean;
            case CHAR_TAG     : return Object::Type::Character;
            case REAL_TAG     : return Object::Type::Real;
            case NATIVE_TAG   : return Object::Type::NativeReference;
            default: throw std::runtime_error{"This should never happen in getType"};
        }
    }

    integer_t getIntegerData() const {
        symbol_id_t uncasted = data(this->_data);
        integer_t unshifted = *reinterpret_cast<integer_t*>(&uncasted);
        integer_t shifted = unshifted >> TYPE_TAG_SIZE;
        return shifted;
    }


    static symbol_id_t pointerData(void* ptr) { 
        return *reinterpret_cast<symbol_id_t*>(&ptr); 
    }

    static HeapObject* getPointerData(symbol_id_t data) { 
        return *reinterpret_cast<HeapObject**>(&data);
    }

    static void* getNativePointerData(symbol_id_t data) { 
        return *reinterpret_cast<void**>(&data);
    }

    static symbol_id_t data(symbol_id_t value) { return value & DATA_TAG; }
    static symbol_id_t type(symbol_id_t value) { return value & TYPE_TAG; }
    static symbol_id_t join(symbol_id_t _data, symbol_id_t _tag) { return data(_data) | type(_tag); }

    void replace(symbol_id_t value, symbol_id_t tag) {
        this->_data = join(data(value), type(tag));
    }

    void replaceTag(symbol_id_t tag) {
        this->_data = join(data(this->_data), type(tag));
    }
};

static_assert(sizeof(Object) == sizeof(integer_t));

enum class HeapObjectType {

};

class HeapObject {
private:
    uinteger_t type;
    uinteger_t allocation_size;
protected:
    HeapObjectType GetType() const;
    uinteger_t GetAllocationSize() const;
    Object GetSlot(uinteger_t index) const;
    void SetSlot(uinteger_t index, Object val);
};

static_assert(sizeof(HeapObject) == sizeof(std::int64_t));
static_assert(sizeof(HeapObject) == sizeof(Object));
static_assert(sizeof(HeapObject) % 8 == 0);

class Pair : public HeapObject {
private:
};

static_assert(sizeof(Pair) == sizeof(HeapObject));

class Vector : public HeapObject {
public:
    Object GetLength() const { return GetSlot(0); }
    Object GetItem(std::uint32_t index) const { return GetSlot(index + 1); }
};

static_assert(sizeof(Vector) == sizeof(HeapObject));

class String : public HeapObject {
private:
    Object GetLength() const { return GetSlot(0); }
    Object GetChar(std::uint32_t index) const { return GetSlot(index + 1); }
};

static_assert(sizeof(String) == sizeof(HeapObject));

class Map : public HeapObject {
private:
    Object GetHead() const { return GetSlot(0); }
    Object GetSize() const { return GetSlot(1); }
public:
    Object Lookup(Object key) const;
    void Insert(Object key, Object value);
};

static_assert(sizeof(Map) == sizeof(HeapObject));

class Envrionment : public HeapObject {
private:
    Object GetLookup() { return GetSlot(0); }
    Object GetOuter() { return GetSlot(1); }

public:
    Map GetLookup() const;
    Object GetOuter() const;
};

static_assert(sizeof(Envrionment) == sizeof(HeapObject));

class Stack : public HeapObject {
private:
    Object GetHead() { return GetSlot(0); }
    Object GetSize() { return GetSlot(1); }

public:
    void Push(Object obj);
    Object Pop();
    Object Size() const;
};

static_assert(sizeof(Stack) == sizeof(HeapObject));

class Frame : public HeapObject {
private:
    Envrionment GetEnv() const;
    Object GetOuter() const;
    Stack GetStack() const;
};

}

#endif // REFACTOR_OBJECT_HH__