#ifndef BYTECODE_H__
#define BYTECODE_H__

#include <cstdint>
#include <string>
#include <vector>

#include "memory_semantic_macros.hh"

enum class BytecodeType {
    LoadLocal,
    StoreLocal,
    JumpIfFalse,
    Jump,
    Invoke,
    LoadTrue,
    LoadFalse,
    LoadNil,
    LoadInteger,
    LoadSymbol,
    LoadCharacter,
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

    NOT_COPYABLE(Bytecode);

    MOVEABLE(Bytecode);

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
public:
    Function(std::uint64_t _arity, std::vector<Bytecode> _bytecode) 
    : arity{_arity}, bytecode{std::move(_bytecode)}
    {}

    ~Function() = default;

    NOT_COPYABLE(Function);

    MOVEABLE(Function);

    uint64_t GetArity() {
        return arity;
    }

    const std::vector<Bytecode>& GetBytecode() {
        return bytecode;
    }
};

class File {
private:
    std::vector<Function> functions;
    std::vector<std::int64_t> integer_constants;
    std::vector<char> character_constants;
    std::vector<std::string> symbol_constants;
public:
    File(std::vector<Function> _functions, 
         std::vector<std::int64_t> _integer_constants,
         std::vector<char> _character_constants,
         std::vector<std::string> _symbol_constants) 
    : functions{std::move(_functions)}
    , integer_constants{std::move(_integer_constants)}
    , character_constants{std::move(_character_constants)}
    , symbol_constants{std::move(_symbol_constants)}
    {}

    ~File() = default;

    MOVEABLE(File);

    NOT_COPYABLE(File);

    const std::vector<Function>& GetFunctions() {
        return functions;
    }

    const std::vector<std::int64_t>& GetIntegerConstants() {
        return integer_constants;
    }

    const std::vector<char>& GetCharacterConstants() {
        return character_constants;
    }

    const std::vector<std::string>& GetSymbolConstants() {
        return symbol_constants;
    }
};

#endif // BYTECODE_H__