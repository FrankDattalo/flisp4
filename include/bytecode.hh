#ifndef BYTECODE_H__
#define BYTECODE_H__

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>
#include <variant>

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
    InvokeExternal,
    LoadUnsigned,
    Pop,
    // Throw,
};

enum class BytecodeArgType {
    None,
    Unsigned,
};

class BytecodeArg {
private:
    std::variant<std::monostate, std::uint64_t> arg;
public:
    explicit BytecodeArg(std::uint64_t _arg): arg{_arg} {}

    BytecodeArg() {}

    ~BytecodeArg() = default;

    COPYABLE(BytecodeArg);
    NOT_MOVEABLE(BytecodeArg);

    template <typename T>
    T Get() const { return std::get<T>(arg); }

    BytecodeArgType Type() const { return static_cast<BytecodeArgType>(arg.index()); }
};

class Bytecode {
private:
    BytecodeType type;
    BytecodeArg arg;
public:
    Bytecode(BytecodeType _type, BytecodeArg _arg)
    : type{_type}, arg{_arg}
    {
        if (ArgType(type) != arg.Type()) {
            std::string msg{"Bytecode initialized with incorrect arg type for bytecode"};
            msg.append(TypeToString(this->type));
            throw std::runtime_error{msg};
        }
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
        return this->arg.Get<std::uint64_t>();
    }

    static BytecodeArgType ArgType(BytecodeType type) {
        switch (type) {
            case BytecodeType::InvokeExternal:
            case BytecodeType::InvokeNative: 
            case BytecodeType::InvokeFunction:
            case BytecodeType::LoadInteger:
            case BytecodeType::JumpIfFalse:
            case BytecodeType::Jump:
            case BytecodeType::LoadLocal:
            case BytecodeType::StoreLocal:
            case BytecodeType::LoadString:
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

    std::string ArgToString() const {
        switch (GetArgType()) {
            case BytecodeArgType::None: return "";
            case BytecodeArgType::Unsigned: return std::to_string(this->GetUnsignedArg());
            default: {
                std::string msg{"Unhandled arg type in ArgToString"};
                msg.append(std::to_string(static_cast<std::uint64_t>(GetArgType())));
                throw std::runtime_error{msg};
            }
        }
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
            case BytecodeType::InvokeExternal: return "InvokeExternal";
            case BytecodeType::LoadUnsigned: return "LoadUnsigned";
            case BytecodeType::LoadTrue: return "LoadTrue";
            case BytecodeType::LoadFalse: return "LoadFalse";
            case BytecodeType::Halt: return "Halt";
            case BytecodeType::LoadNil: return "LoadNil";
            case BytecodeType::Return: return "Return";
            case BytecodeType::Pop:  return "Pop";
            // case BytecodeType::Throw: return "Throw";
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
        if (str == "InvokeExternal") return BytecodeType::InvokeExternal;
        if (str == "LoadUnsigned") return BytecodeType::LoadUnsigned;
        if (str == "Pop") return BytecodeType::Pop;
        // if (str == "Throw") return BytecodeType::Throw;
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

template <typename T>
class TaggedConstant {
private:
    T data;
public:
    explicit TaggedConstant(T _data): data{_data} {}

    ~TaggedConstant() = default;

    NOT_COPYABLE(TaggedConstant);
    MOVEABLE(TaggedConstant);

    constexpr const T& Get() const { return data; };
};

using IntegerConstant = TaggedConstant<std::int64_t>;
using StringConstant = TaggedConstant<std::string>;

enum class ConstantType {
    Integer,
    String,
};

class Constant {
private:
    std::variant<IntegerConstant, StringConstant> value;
public:
    template<typename T>
    explicit Constant(T _value): value{std::move(_value)} {}

    ~Constant() = default;

    NOT_COPYABLE(Constant);
    MOVEABLE(Constant);

    template<typename T>
    decltype(auto) Get() const { return std::get<T>(value).Get(); }

    ConstantType Type() const { return static_cast<ConstantType>(value.index()); }

    std::string ToString() const {
        std::stringstream stream;
        switch (Type()) {
            case ConstantType::Integer: {
                stream << "IntegerConstant{" << Get<IntegerConstant>() << "}";
                break;
            }
            case ConstantType::String: {
                stream << "StringConstant{" << Get<StringConstant>() << "}";
                break;
            }
            default: {
                std::string msg{"Unhandled constant type in ToString: "};
                msg.append(std::to_string(static_cast<std::uint8_t>(Type())));
                throw std::runtime_error{msg};
            }
        }
        return stream.str();
    }
};

class File {
private:
    std::uint64_t version;
    std::vector<Function> functions;
    std::vector<Constant> constants;
public:
    File(std::uint64_t _version,
         std::vector<Function> _functions, 
         std::vector<Constant> _constants) 
    : version{_version}
    , functions{std::move(_functions)}
    , constants{std::move(_constants)}
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

    const std::vector<Constant>& GetConstants() const {
        return constants;
    }
};

#endif // BYTECODE_H__