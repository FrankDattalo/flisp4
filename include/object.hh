#ifndef OBJECT_HH__
#define OBJECT_HH__

#include <cstdint>
#include <limits>

#include "memory_semantic_macros.hh"

enum class ObjectType {
    Nil,
    ObjectHeader,
    GcForward,
    Integer,
    Boolean,
    Reference,
    Symbol,
    Character,
    __DO_NOT_ADD_AFTER_THIS__
};

// ensure that we don't define more than 8 object types
static_assert(0b1000 >= static_cast<int>(ObjectType::__DO_NOT_ADD_AFTER_THIS__));

class Object {
private:
    static const std::uint64_t TYPE_MASK = 0b1111;
    static const std::uint64_t TYPE_MASK_BITS = 4;
    static const std::uint64_t DATA_MASK = ~TYPE_MASK;
    static const std::int64_t INTEGER_MAX = std::numeric_limits<std::int64_t>::max() >> TYPE_MASK_BITS;
    static const std::int64_t INTEGER_MIN = std::numeric_limits<std::int64_t>::min() >> TYPE_MASK_BITS;
    static const std::uint64_t UNSIGNED_MAX = std::numeric_limits<std::uint64_t>::max() >> TYPE_MASK_BITS;
    union {
        Object*       reference;
        std::int64_t  integer;
        std::uint64_t unsigned_integer;
    };
public:
    Object() {
        SetNil();
        // debug
        checkType(ObjectType::Nil);
    }

    ~Object() = default;

    COPYABLE(Object);

    NOT_MOVEABLE(Object);

    bool IsNil() {
        return getType() == ObjectType::Nil;
    }

    void SetNil() {
        setUnsignedData(0);
        setType(ObjectType::Nil);
    }

    bool IsReference() {
        return getType() == ObjectType::Reference;
    }

    void SetReference(Object* reference) {
        setPointerData(reference);
        setType(ObjectType::Reference);
    }

    Object* GetReference() {
        checkType(ObjectType::Reference);
        return getPointerData();
    }

    void SetBoolean(bool value) {
        setIntegerData(value);
        setType(ObjectType::Boolean);
    }

    bool GetBoolean() {
        checkType(ObjectType::Boolean);
        return getIntegerData();
    }

    void SetInteger(std::int64_t value) {
        setIntegerData(value);
        setType(ObjectType::Integer);
    }

    std::int64_t GetInteger() {
        checkType(ObjectType::Integer);
        return getIntegerData();
    }

    bool IsGcForward() {
        return getType() == ObjectType::GcForward;
    }

    void SetGcForward(Object* fwd) {
        setPointerData(reference);
        setType(ObjectType::Reference);
    }

    Object* GetGcForward() {
        checkType(ObjectType::GcForward);
        return getPointerData();
    }

    bool IsObjectHeader() {
        return getType() == ObjectType::ObjectHeader;
    }

    void SetObjectHeader(std::uint64_t elements) {
        setUnsignedData(elements);
        setType(ObjectType::ObjectHeader);
    }

    std::uint64_t GetAllocationSize() {
        checkType(ObjectType::ObjectHeader);
        return getUnsignedData();
    }

    ObjectType GetObjectType() {
        return getType();
    }

private:
    void checkType(ObjectType expected) {
        if (getType() == expected) {
            return;
        }
        std::string error_message{"Expected object of type "};
        error_message.append(std::to_string(static_cast<std::size_t>(expected)));
        error_message.append(", but was ");
        error_message.append(std::to_string(static_cast<std::size_t>(getType())));
        throw std::runtime_error{error_message};
    }

    ObjectType getType() {
        ObjectType result = static_cast<ObjectType>(TYPE_MASK & this->unsigned_integer);
        if (result >= ObjectType::__DO_NOT_ADD_AFTER_THIS__) {
            throw std::runtime_error{std::string{"Memory corruption, unknown object type"}};
        }
        return result;
    }

    void setPointerData(Object* data) {
        checkAlignment(data);
        this->unsigned_integer = reinterpret_cast<std::uint64_t>(data) & DATA_MASK;
    }

    void checkAlignment(Object* data) {
        std::uint64_t casted = reinterpret_cast<std::uint64_t>(data) & TYPE_MASK;
        if (casted != 0) {
            throw std::runtime_error{std::string{"Pointer alignment error"}};
        }
    }

    Object* getPointerData() {
        return reinterpret_cast<Object*>(this->unsigned_integer & DATA_MASK);
    }

    void setUnsignedData(std::uint64_t data) {
        if (data > Object::UNSIGNED_MAX) {
            throw std::runtime_error{std::string{"Unsigned integer overflow"}};
        }
        this->unsigned_integer = data << TYPE_MASK_BITS;
    }

    std::uint64_t getUnsignedData() {
        return this->unsigned_integer >> TYPE_MASK_BITS;
    }

    void setIntegerData(std::int64_t data) {
        if (data > Object::INTEGER_MAX) {
            throw std::runtime_error{std::string{"Integer overflow"}};
        }
        if (data < Object::INTEGER_MIN) {
            throw std::runtime_error{std::string{"Integer underflow"}};
        }
        this->integer = data << TYPE_MASK_BITS;
    }

    std::int64_t getIntegerData() {
        return this->integer >> TYPE_MASK_BITS;
    }

    void setType(ObjectType type) {
        // debug
        if (type >= ObjectType::__DO_NOT_ADD_AFTER_THIS__) {
            throw std::runtime_error{std::string{"Type too big to set"}};
        }
        this->unsigned_integer |= static_cast<std::uint64_t>(type);
    }
};

static_assert(sizeof(Object) == sizeof(std::uint64_t));

#endif // OBJECT_HH__