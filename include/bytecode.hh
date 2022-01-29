#ifndef BYTECODE_H__
#define BYTECODE_H__

#include <cstdint>
#include <string>
#include <vector>

#include "memory_semantic_macros.hh"

enum class BytecodeType {
    Halt,
    JumpIfFalse,
    Jump,
    LoadNil,
    Return,
    LoadLocal,
    StoreLocal,
    LoadInteger,
    LoadString,
    LoadTrue,
    LoadFalse,
    InvokeNative,
    InvokeFunction,
    LoadUnsigned,
    Pop
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

    MOVEABLE(Bytecode);

    BytecodeType GetType() const {
        return type;
    }

    std::uint64_t GetArg() const {
        bool hasArg = HasArg(this->type);
        if (!hasArg) {
            throw std::runtime_error{std::string{"GetArg called on argless bytecode"}};
        }
        return arg;
    }

    static bool HasArg(BytecodeType type) {
        switch (type) {
            case BytecodeType::MakeFunction:
            case BytecodeType::LoadLocal:
            case BytecodeType::StoreLocal:
            case BytecodeType::JumpIfFalse:
            case BytecodeType::Jump:
            case BytecodeType::Invoke:
            case BytecodeType::LoadInteger:
            case BytecodeType::LoadSymbol:
            case BytecodeType::LoadCharacter:
            case BytecodeType::LoadField:
            case BytecodeType::StoreField: {
                return true;
            }
            case BytecodeType::Halt:
            case BytecodeType::Return:
            case BytecodeType::LoadTrue:
            case BytecodeType::LoadFalse:
            case BytecodeType::LoadNil: {
                return false;
            }
            default: {
                std::string msg{"Unhandled bytecode in HasArg: "};
                msg.append(std::to_string(static_cast<uint64_t>(type)));
                throw std::runtime_error{msg};
            }
        }
    }

    static std::string TypeToString(BytecodeType type) {
        switch (type) {
            case BytecodeType::LoadLocal: return "LoadLocal";
            case BytecodeType::StoreLocal: return "StoreLocal";
            case BytecodeType::JumpIfFalse: return "JumpIfFalse";
            case BytecodeType::Jump: return "Jump";
            case BytecodeType::Invoke: return "Invoke";
            case BytecodeType::LoadInteger: return "LoadInteger";
            case BytecodeType::LoadSymbol: return "LoadSymbol";
            case BytecodeType::LoadCharacter: return "LoadCharacter";
            case BytecodeType::LoadField: return "LoadField";
            case BytecodeType::StoreField: return "StoreField";
            case BytecodeType::LoadTrue: return "LoadTrue";
            case BytecodeType::LoadFalse: return "LoadFalse";
            case BytecodeType::LoadNil: return "LoadNil";
            case BytecodeType::Return: return "Return";
            case BytecodeType::Halt: return "Halt";
            case BytecodeType::MakeFunction: return "MakeFunction";
            default: {
                std::string msg{"Unhandled bytecode in TypeToString: "};
                msg.append(std::to_string(static_cast<uint64_t>(type)));
                throw std::runtime_error{msg};
            }
        }
    }
};

class Function {
private:
    std::uint64_t arity;
    std::uint64_t locals;
    std::vector<Bytecode> bytecode;
public:
    Function(std::uint64_t _arity, std::uint64_t _locals, std::vector<Bytecode> _bytecode) 
    : arity{_arity}, locals{_locals}, bytecode{std::move(_bytecode)}
    {}

    ~Function() = default;

    NOT_COPYABLE(Function);

    MOVEABLE(Function);

    std::uint64_t GetArity() const {
        return arity;
    }

    std::uint64_t GetLocals() const {
        return locals;
    }

    const std::vector<Bytecode>& GetBytecode() const {
        return bytecode;
    }
};

class File {
private:
    std::uint64_t version;
    std::vector<Function> functions;
    std::vector<std::int64_t> integer_constants;
    std::vector<char> character_constants;
    std::vector<std::string> symbol_constants;
public:
    File(std::uint64_t _version,
         std::vector<Function> _functions, 
         std::vector<std::int64_t> _integer_constants,
         std::vector<char> _character_constants,
         std::vector<std::string> _symbol_constants) 
    : version{_version}
    , functions{std::move(_functions)}
    , integer_constants{std::move(_integer_constants)}
    , character_constants{std::move(_character_constants)}
    , symbol_constants{std::move(_symbol_constants)}
    {}

    ~File() = default;

    MOVEABLE(File);

    NOT_COPYABLE(File);

    std::uint64_t GetVersion() const {
        return version;
    }

    const std::vector<Function>& GetFunctions() const {
        return functions;
    }

    const std::vector<std::int64_t>& GetIntegerConstants() const {
        return integer_constants;
    }

    const std::vector<char>& GetCharacterConstants() const {
        return character_constants;
    }

    const std::vector<std::string>& GetSymbolConstants() const {
        return symbol_constants;
    }
};

#endif // BYTECODE_H__