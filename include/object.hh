#ifndef OBJECT_HH__
#define OBJECT_HH__

#include <cstdint>
#include <limits>

#include "memory_semantic_macros.hh"
#include "bytecode.hh"

enum class ObjectType {
    Nil, 
    Reference, 
    Integer, 
    Boolean, 
    Symbol,
    Real,
    Character,
    ObjectHeader,
    GcForward,
    FunctionReference,
};

class Object {
private:
    ObjectType type;
    union {
        const Function* function_reference;
        Object*         reference;
        std::int64_t    integer;
        std::uint64_t   unsigned_integer;
        double          real;
    };
public:
    Object() {
        SetNil();
    }

    ~Object() = default;

    COPYABLE(Object);

    NOT_MOVEABLE(Object);

    bool IsType(ObjectType type) {
        return this->type == type;
    }

    void SetNil() {
        this->type = ObjectType::Nil;
    }

    void SetReference(Object* reference) {
        this->type = ObjectType::Reference;
        this->reference = reference;
    }

    Object* GetReference() {
        checkType(ObjectType::Reference);
        return this->reference;
    }

    void SetBoolean(bool value) {
        this->type = ObjectType::Boolean;
        this->integer = value;
    }

    bool GetBoolean() {
        checkType(ObjectType::Boolean);
        return this->integer;
    }

    void SetInteger(std::int64_t value) {
        this->type = ObjectType::Integer;
        this->integer = value;
    }

    std::int64_t GetInteger() {
        checkType(ObjectType::Integer);
        return this->integer;
    }

    void SetCharacter(char value) {
        this->type = ObjectType::Character;
        this->integer = value;
    }

    char GetCharacter() {
        checkType(ObjectType::Character);
        return this->integer;
    }

    void SetSymbol(std::uint64_t value) {
        this->type = ObjectType::Symbol;
        this->unsigned_integer = value;
    }

    std::uint64_t GetSymbol() {
        checkType(ObjectType::Symbol);
        return this->unsigned_integer;
    }

    void SetGcForward(Object* fwd) {
        this->type = ObjectType::GcForward;
        this->reference = reference;
    }

    Object* GetGcForward() {
        checkType(ObjectType::GcForward);
        return this->reference;
    }

    void SetObjectHeader(std::uint64_t allocation_size) {
        this->type = ObjectType::ObjectHeader;
        this->unsigned_integer = allocation_size;
    }

    std::uint64_t GetAllocationSize() {
        checkType(ObjectType::ObjectHeader);
        return this->unsigned_integer;
    }

    ObjectType GetType() {
        return this->type;
    }

    void SetFunctionReference(const Function* fn) {
        this->type = ObjectType::FunctionReference;
        this->function_reference = fn;
    }

    const Function* GetFunctionReference() {
        checkType(ObjectType::FunctionReference);
        return this->function_reference;
    }

    void SetReal(double value) {
        this->type = ObjectType::Real;
        this->real = value;
    }

    double GetReal() {
        checkType(ObjectType::Real);
        return this->real;
    }

private:
    void checkType(ObjectType expected) {
        if (this->type == expected) {
            return;
        }
        std::string error_message{"Expected object of type "};
        error_message.append(std::to_string(static_cast<std::size_t>(expected)));
        error_message.append(", but was ");
        error_message.append(std::to_string(static_cast<std::size_t>(this->type)));
        throw std::runtime_error{error_message};
    }
};

static_assert(sizeof(Object) == 2 * sizeof(std::uint64_t));

#endif // OBJECT_HH__