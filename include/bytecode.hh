#ifndef BYTECODE_H__
#define BYTECODE_H__

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>

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

enum class BytecodeArgType {
    Signed,
    Unsigned,
    None
};

static_assert(sizeof(BytecodeType) <= sizeof(std::uint32_t));

class BytecodeArg {
private:
    union {
        std::uint64_t unsigned_int;
        std::int64_t signed_int;
    };
public:
    explicit BytecodeArg(std::uint64_t _val): unsigned_int{_val} {}
    explicit BytecodeArg(std::int64_t _val): signed_int{_val} {}

    BytecodeArg() : unsigned_int{0} {}

    COPYABLE(BytecodeArg);

    NOT_MOVEABLE(BytecodeArg);

    std::uint64_t UnsignedInt() const {
        return unsigned_int;
    }

    std::int64_t SignedInt() const {
        return signed_int;
    }
};

static_assert(sizeof(BytecodeArg) == sizeof(std::uint64_t));

class Bytecode {
private:
    BytecodeType type;
    BytecodeArg arg;
public:
    Bytecode(BytecodeType _type, BytecodeArgType _arg_type, BytecodeArg _arg)
    : type{_type}, arg{_arg}
    {
        checkArg(_arg_type);
    }

    Bytecode(BytecodeType _type)
    : type{_type}
    {
        checkArg(BytecodeArgType::None);
    }

    ~Bytecode() = default;

    COPYABLE(Bytecode);

    MOVEABLE(Bytecode);

    BytecodeType GetType() const {
        return type;
    }

    BytecodeArgType GetArgType() const {
        return ArgType(this->type);
    }

    bool IsHasArg() const {
        return HasArg(this->type);
    }

    const std::string GetTypeToString() const {
        return TypeToString(this->type);
    }

    std::uint64_t GetUnsignedArg() const {
        checkArg(BytecodeArgType::Unsigned);
        return this->arg.UnsignedInt();
    }

    std::uint64_t GetSignedArg() const {
        checkArg(BytecodeArgType::Signed);
        return this->arg.SignedInt();
    }
private:
    void checkArg(BytecodeArgType requested) const {
        if (ArgType(this->type) != requested) {
            std::string msg{"GetXXXArg called on an invalid bytecode: "};
            msg.append(TypeToString(this->type));
            throw std::runtime_error{msg};
        }
    }
public:
    static BytecodeArgType ArgType(BytecodeType type) {
        switch (type) {
            case BytecodeType::LoadInteger: {
                return BytecodeArgType::Signed;
            }
            case BytecodeType::JumpIfFalse:
            case BytecodeType::Jump:
            case BytecodeType::LoadLocal:
            case BytecodeType::StoreLocal:
            case BytecodeType::LoadString:
            case BytecodeType::InvokeNative:
            case BytecodeType::InvokeFunction:
            case BytecodeType::LoadUnsigned: {
                return BytecodeArgType::Unsigned;
            }
            case BytecodeType::LoadTrue:
            case BytecodeType::LoadFalse:
            case BytecodeType::Halt:
            case BytecodeType::LoadNil:
            case BytecodeType::Return:
            case BytecodeType::Pop: {
                return BytecodeArgType::None;
            }
            default: {
                std::string msg{"Unhandled bytecode in HasArg: "};
                msg.append(std::to_string(static_cast<uint64_t>(type)));
                throw std::runtime_error{msg};
            }
        }
    }

    static bool HasArg(BytecodeType type) {
        return ArgType(type) != BytecodeArgType::None;
    }

    static std::string TypeToString(BytecodeType type) {
        switch (type) {
            case BytecodeType::JumpIfFalse: return "JumpIfFalse";
            case BytecodeType::Jump: return "Jump";
            case BytecodeType::LoadLocal: return "LoadLocal";
            case BytecodeType::StoreLocal: return "StoreLocal";
            case BytecodeType::LoadInteger: return "LoadInteger";
            case BytecodeType::LoadString: return "LoadString";
            case BytecodeType::InvokeNative: return "InvokeNative";
            case BytecodeType::InvokeFunction: return "InvokeFunction";
            case BytecodeType::LoadUnsigned: return "LoadUnsigned";
            case BytecodeType::LoadTrue: return "LoadTrue";
            case BytecodeType::LoadFalse: return "LoadFalse";
            case BytecodeType::Halt: return "Halt";
            case BytecodeType::LoadNil: return "LoadNil";
            case BytecodeType::Return: return "Return";
            case BytecodeType::Pop:  return "Pop";
            default: {
                std::string msg{"Unhandled bytecode in TypeToString: "};
                msg.append(std::to_string(static_cast<uint64_t>(type)));
                throw std::runtime_error{msg};
            }
        }
    }

    static BytecodeType TypeFromString(const std::string & str) {
        if (str == "Halt") return BytecodeType::Halt;
        if (str == "JumpIfFalse") return BytecodeType::JumpIfFalse;
        if (str == "Jump") return BytecodeType::Jump;
        if (str == "LoadNil") return BytecodeType::LoadNil;
        if (str == "Return") return BytecodeType::Return;
        if (str == "LoadLocal") return BytecodeType::LoadLocal;
        if (str == "StoreLocal") return BytecodeType::StoreLocal;
        if (str == "LoadInteger") return BytecodeType::LoadInteger;
        if (str == "LoadString") return BytecodeType::LoadString;
        if (str == "LoadTrue") return BytecodeType::LoadTrue;
        if (str == "LoadFalse") return BytecodeType::LoadFalse;
        if (str == "InvokeNative") return BytecodeType::InvokeNative;
        if (str == "InvokeFunction") return BytecodeType::InvokeFunction;
        if (str == "LoadUnsigned") return BytecodeType::LoadUnsigned;
        if (str == "Pop") return BytecodeType::Pop;
        std::string msg{"Unhandled bytecode in TypeFromString: "};
        msg.append(str);
        throw std::runtime_error{msg};
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
    std::vector<std::string> string_constants;
public:
    File(std::uint64_t _version,
         std::vector<Function> _functions, 
         std::vector<std::string> _string_constants) 
    : version{_version}
    , functions{std::move(_functions)}
    , string_constants{std::move(_string_constants)}
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

    const std::vector<std::string>& GetStringConstants() const {
        return string_constants;
    }
};

#endif // BYTECODE_H__