#ifndef BYTECODE_H__
#define BYTECODE_H__

#include <cstdint>
#include <vector>

#include "memory_semantic_macros.hh"

enum class BytecodeType {
    LoadLocal,
    StoreLocal,
    JumpIfFalse,
    Jump,
    Invoke,
    LoadConstant,
    LoadField,
    StoreField,
};

static_assert(sizeof(BytecodeType) <= sizeof(std::uint32_t));

class Bytecode {
private:
    BytecodeType type;
    std::uint64_t arg;
public:
    Bytecode(BytecodeType _type, std::uint64_t _arg)
    : type{_type}, arg{_arg}
    {}

    ~Bytecode() = default;

    COPYABLE(Bytecode);

    NOT_MOVEABLE(Bytecode);

    BytecodeType GetType() {
        return type;
    }

    std::uint64_t GetArg() {
        return arg;
    }
};

class Function {
private:
    std::uint64_t arity;
    std::vector<Bytecode> bytecode;
};

enum class ConstantType {
    Character,
    Integer,
    Symbol,
};

class Constant {
private:
    ConstantType type;
    int64_t integer;
    char character;
    std::string symbol;
};

class File {
private:
    std::vector<Function> functions;
    std::vector<Constant> constants;
};

#endif // BYTECODE_H__