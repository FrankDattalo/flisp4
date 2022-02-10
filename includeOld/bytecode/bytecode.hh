#ifndef BYTECODE_H__
#define BYTECODE_H__

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>
#include <variant>

#include "util/debug.hh"
#include "util/memory_semantic_macros.hh"

namespace bytecode {

#define PER_BYTECODE_TYPE(V) \
    V(Halt) \
    V(JumpIfFalse) \
    V(Jump) \
    V(LoadNil) \
    V(Return) \
    V(LoadLocal) \
    V(StoreLocal) \
    V(LoadInteger) \
    V(LoadString) \
    V(LoadTrue) \
    V(LoadFalse) \
    V(Invoke) \
    V(Pop)

enum class BytecodeType {
#define COMMA(val) val,
    PER_BYTECODE_TYPE(COMMA)
#undef COMMA
};

#define PER_BYTECODE_ARG_TYPE(V) \
    V(None) \
    V(Unsigned) 

enum class BytecodeArgType {
#define COMMA(val) val,
    PER_BYTECODE_ARG_TYPE(COMMA)
#undef COMMA
};

class BytecodeArg;

class BytecodeArgVisitor {
public:
#define DEFINE_VISITOR(val) virtual void On##val(const BytecodeArg& bcArg) = 0;
    PER_BYTECODE_ARG_TYPE(DEFINE_VISITOR)
#undef DEFINE_VISITOR
};

class BytecodeArgTypeVisitor {
public:
#define DEFINE_VISITOR(val) virtual void On##val(BytecodeArgType bcArg) = 0;
    PER_BYTECODE_ARG_TYPE(DEFINE_VISITOR)
#undef DEFINE_VISITOR
};

class BytecodeArg {
friend class Bytecode;
private:
    // these need to map the delcaration order of the bytecode arg types
    std::variant<std::monostate, std::uint64_t> arg;
public:
    explicit BytecodeArg(std::uint64_t _arg): arg{_arg} {}

    BytecodeArg() {}

    ~BytecodeArg() = default;

    COPYABLE(BytecodeArg);
    NOT_MOVEABLE(BytecodeArg);

private:
    template <typename T>
    T Get() const { return std::get<T>(arg); }

    BytecodeArgType Type() const { return static_cast<BytecodeArgType>(arg.index()); }
    public:

    void Visit(BytecodeArgVisitor& visitor) const {
        #define ADD_VISITOR_CALL(val) case BytecodeArgType::val: { visitor.On##val(*this); return; }
        switch (Type()) {
            PER_BYTECODE_ARG_TYPE(ADD_VISITOR_CALL)
            default: throw std::runtime_error{"This should never happen in BytecodeArg Visit"};
        }
        #undef ADD_VISITOR_CALL
    }

    static void VisitType(BytecodeArgType type, BytecodeArgTypeVisitor& visitor) {
        #define ADD_VISITOR_CALL(val) case BytecodeArgType::val: { visitor.On##val(type); return; }
        switch (type) {
            PER_BYTECODE_ARG_TYPE(ADD_VISITOR_CALL)
            default: throw std::runtime_error{"This should never happen in BytecodeArg VisitType"};
        }
        #undef ADD_VISITOR_CALL
    }
};

class Bytecode;

class BytecodeVisitor {
public:
#define DEFINE_VISITOR(val) virtual void On##val(const bytecode::Bytecode& bc) = 0;
    PER_BYTECODE_TYPE(DEFINE_VISITOR)
#undef DEFINE_VISITOR
};

class BytecodeTypeVisitor {
public:
#define DEFINE_VISITOR(val) virtual void On##val(BytecodeType bc) = 0;
    PER_BYTECODE_TYPE(DEFINE_VISITOR)
#undef DEFINE_VISITOR
};

class Bytecode {
private:
    BytecodeType type;
    BytecodeArg arg;
public:
    Bytecode(BytecodeType _type, BytecodeArg _arg)
    : type{_type}, arg{_arg}
    {
        checkArgType();
    }

    ~Bytecode() = default;

    COPYABLE(Bytecode);

    MOVEABLE(Bytecode);

    void Visit(BytecodeVisitor& visitor) const {
        #define ADD_VISITOR_CALL(val) case BytecodeType::val: { visitor.On##val(*this); return; }
        switch (GetType()) {
            PER_BYTECODE_TYPE(ADD_VISITOR_CALL)
            default: throw std::runtime_error{"This should never happen in Bytecode Visit"};
        }
        #undef ADD_VISITOR_CALL
    }

    void VisitArg(BytecodeArgVisitor& visitor) const {
        arg.Visit(visitor);
    }

    static void VisitType(BytecodeType type, BytecodeTypeVisitor& visitor) {
        #define ADD_VISITOR_CALL(val) case BytecodeType::val: { visitor.On##val(type); return; }
        switch (type) {
            PER_BYTECODE_TYPE(ADD_VISITOR_CALL)
            default: throw std::runtime_error{"This should never happen Bytecode VisitType"};
        }
        #undef ADD_VISITOR_CALL
    }

    static void VisitArgType(BytecodeType type, BytecodeArgTypeVisitor& visitor) {
        DEBUGLN("Visiting arg type for bytecode type " << std::to_string(static_cast<std::uint8_t>(type)));
        BytecodeArgType argType = ArgType(type);
        DEBUGLN("Arg type for bytecode type is " << std::to_string(static_cast<std::uint8_t>(argType)));
        BytecodeArg::VisitType(argType, visitor);
    }

private:
    BytecodeType GetType() const {
        return type;
    }

    void checkArgType() {

        BytecodeArgType argType;

        struct Visitor : public BytecodeArgVisitor {
            BytecodeArgType& argType;

            Visitor(BytecodeArgType& _t): argType{_t} {}

            void OnNone(const BytecodeArg&) override {
                argType = BytecodeArgType::None;
            }
            void OnUnsigned(const BytecodeArg&) override {
                argType = BytecodeArgType::Unsigned;
            }

        } visitor(argType);

        arg.Visit(visitor);

        if (ArgType(type) != argType) {
            std::string msg{"Bytecode initialized with incorrect arg type for bytecode"};
            msg.append(TypeToString(this->type));
            throw std::runtime_error{msg};
        }
    }
public:

    const std::string GetTypeToString() const {
        return TypeToString(this->type);
    }

    std::uint64_t GetUnsignedArg() const {
        return this->arg.Get<std::uint64_t>();
    }

private:
    BytecodeArgType GetArgType() const {
        return ArgType(this->type);
    }

    static BytecodeArgType ArgType(BytecodeType type) {
        BytecodeArgType argType;

        struct Visitor : public BytecodeTypeVisitor {
            BytecodeArgType& argType;

            Visitor(BytecodeArgType& _t): argType{_t} {}

            void OnJumpIfFalse(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnJump(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnLoadLocal(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnStoreLocal(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnLoadInteger(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnLoadString(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnInvoke(BytecodeType) override { argType = BytecodeArgType::Unsigned; }
            void OnPop(BytecodeType) override { argType = BytecodeArgType::None; }
            void OnHalt(BytecodeType) override { argType = BytecodeArgType::None; }
            void OnLoadNil(BytecodeType) override { argType = BytecodeArgType::None; }
            void OnReturn(BytecodeType) override { argType = BytecodeArgType::None; }
            void OnLoadTrue(BytecodeType) override { argType = BytecodeArgType::None; }
            void OnLoadFalse(BytecodeType) override { argType = BytecodeArgType::None; }

        } visitor(argType);

        VisitType(type, visitor);

        return argType;
    }
public:

    std::string ArgToString() const {

        std::string str;

        struct Visitor : public BytecodeArgVisitor {
            std::string& str;
            const Bytecode& self;

            Visitor(std::string& _str, const Bytecode& _self): str{_str}, self{_self} {}

            void OnNone(const BytecodeArg& arg) override { str.assign(""); }
            void OnUnsigned(const BytecodeArg& arg) override { str.assign(std::to_string(self.GetUnsignedArg())); }

        } visitor(str, *this);

        arg.Visit(visitor);

        return str;
    }

    static std::string TypeToString(BytecodeType type) {
        std::string str;

        struct Visitor : public BytecodeTypeVisitor {
            std::string& str;

            Visitor(std::string& _str): str{_str} {}

            #define ADD_ENTRY(val) void On##val(BytecodeType bc) override { str.assign(#val); }
            PER_BYTECODE_TYPE(ADD_ENTRY)
            #undef ADD_ENTRY

        } visitor(str);

        VisitType(type, visitor);

        return str;
    }

    static BytecodeType TypeFromString(const std::string& str) {
        #define ADD_ENTRY(val) if (str == #val) { return BytecodeType::val; }
        PER_BYTECODE_TYPE(ADD_ENTRY)
        #undef ADD_ENTRY
        std::string msg{"Unknown bytecode type in TypeFromString: "};
        msg.append(str);
        throw std::runtime_error{msg};
    }
};

class Function {
private:
    std::string name;
    std::uint64_t arity;
    std::uint64_t locals;
    std::vector<Bytecode> bytecode;
public:
    Function(
        std::string _name,
        std::uint64_t _arity, 
        std::uint64_t _locals, 
        std::vector<Bytecode> _bytecode) 
    : name{std::move(_name)}
    , arity{_arity}
    , locals{_locals}
    , bytecode{std::move(_bytecode)}
    {}

    ~Function() = default;

    NOT_COPYABLE(Function);

    MOVEABLE(Function);

    std::uint64_t GetArity() const { return arity; }

    std::uint64_t GetLocals() const { return locals; }

    const std::string& GetName() const { return name; }

    const std::vector<Bytecode>& GetBytecode() const { return bytecode; }
};

template <typename T>
class TaggedConstant {
private:
    T data;
public:
    explicit TaggedConstant(T _data): data{std::move(_data)} {}

    ~TaggedConstant() = default;

    NOT_COPYABLE(TaggedConstant);
    MOVEABLE(TaggedConstant);

    constexpr const T& Get() const { return data; };
};

class Invocation {
private:
    std::uint64_t module_name_index;
    std::uint64_t function_name_index;
    std::uint64_t argument_count;

public:
    Invocation(
        std::uint64_t _module_name_index, 
        std::uint64_t _function_name_index,
        std::uint64_t _argument_count)
    : module_name_index{_module_name_index}
    , function_name_index{_function_name_index}
    , argument_count{_argument_count}
    {}

    ~Invocation() = default;

    NOT_COPYABLE(Invocation);

    MOVEABLE(Invocation);

    std::uint64_t GetModuleNameIndex() const { return module_name_index; }
    std::uint64_t GetFunctionNameIndex() const { return function_name_index; }
    std::uint64_t GetArgumentCount() const { return argument_count; }
};

using IntegerConstant = TaggedConstant<std::int64_t>;
using StringConstant = TaggedConstant<std::string>;
using InvocationConstant = TaggedConstant<Invocation>;

#define PER_CONSTANT_TYPE(V) \
    V(Integer) \
    V(String) \
    V(Invocation)

enum class ConstantType {
#define COMMA(val) val,
    PER_CONSTANT_TYPE(COMMA)
#undef COMMA
};

class Constant;

class ConstantVisitor {
public:
#define DEFINE_VISITOR(val) virtual void On##val(const Constant& constant) = 0;
    PER_CONSTANT_TYPE(DEFINE_VISITOR)
#undef DEFINE_VISITOR
};

class ConstantTypeVisitor {
public:
#define DEFINE_VISITOR(val) virtual void On##val(ConstantType type) = 0;
    PER_CONSTANT_TYPE(DEFINE_VISITOR)
#undef DEFINE_VISITOR
};

class Constant {
private:
    std::variant<IntegerConstant, StringConstant, InvocationConstant> value;
public:
    template<typename T>
    explicit Constant(T _value): value{std::move(_value)} {}

    ~Constant() = default;

    NOT_COPYABLE(Constant);
    MOVEABLE(Constant);

private:
    template<typename T>
    decltype(auto) Get() const { return std::get<T>(value).Get(); }
public:

    std::int64_t GetIntegerConstant() const { return Get<IntegerConstant>(); }

    const std::string& GetStringConstant() const { return Get<StringConstant>(); }

    const Invocation& GetInvocationConstant() const { return Get<InvocationConstant>(); }

    void Visit(ConstantVisitor& visitor) const {
        #define ADD_VISITOR_CALL(val) case ConstantType::val: { visitor.On##val(*this); return; }
        switch (Type()) {
            PER_CONSTANT_TYPE(ADD_VISITOR_CALL)
            default: throw std::runtime_error{"This should never happen in Constant Visit"};
        }
        #undef ADD_VISITOR_CALL
    }

    static void VisitType(ConstantType type, ConstantTypeVisitor& visitor) {
        #define ADD_VISITOR_CALL(val) case ConstantType::val: { visitor.On##val(type); return; }
        switch (type) {
            PER_CONSTANT_TYPE(ADD_VISITOR_CALL)
            default: throw std::runtime_error{"This should never happen Constant VisitType"};
        }
        #undef ADD_VISITOR_CALL
    }

private:
    ConstantType Type() const { return static_cast<ConstantType>(value.index()); }
public:

    std::string ToString() const {
        std::stringstream stream;

        struct Visitor : public ConstantVisitor {
            std::stringstream& stream;

            Visitor(std::stringstream& _stream): stream{_stream} {}

            void OnInteger(const Constant& constant) override {
                stream << "IntegerConstant{" << constant.GetIntegerConstant() << "}";
            }

            void OnString(const Constant& constant) override {
                stream << "StringConstant{" << constant.GetStringConstant() << "}";
            }

            void OnInvocation(const Constant& constant) override {
                stream << "InvocationConstant{" 
                    << constant.GetInvocationConstant().GetModuleNameIndex() << " "
                    << constant.GetInvocationConstant().GetFunctionNameIndex() << " "
                    << constant.GetInvocationConstant().GetArgumentCount() << "}";
            }

        } visitor(stream);

        Visit(visitor);

        return stream.str();
    }
};

class File {
private:
    std::string module_name;
    std::vector<std::string> import_names;
    std::vector<std::string> export_names;
    std::uint64_t version;
    std::vector<Function> functions;
    std::vector<Constant> constants;
public:
    File() {
        version = 0;
    }

    File(
        std::uint64_t _version,
        std::string _module_name,
        std::vector<std::string> _import_names,
        std::vector<std::string> _export_names,
        std::vector<Function> _functions, 
        std::vector<Constant> _constants) 
    : version{_version}
    , module_name{std::move(_module_name)}
    , import_names{std::move(_import_names)}
    , export_names{std::move(_export_names)}
    , functions{std::move(_functions)}
    , constants{std::move(_constants)}
    {}

    ~File() = default;

    MOVEABLE(File);

    NOT_COPYABLE(File);

    std::uint64_t GetVersion() const { return version; }
    const std::string& GetModuleName() const { return module_name; }
    const std::vector<std::string>& GetImportNames() const { return import_names; }
    const std::vector<std::string>& GetExportNames() const { return export_names; }
    const std::vector<Function>& GetFunctions() const { return functions; }
    const std::vector<Constant>& GetConstants() const { return constants; }
};

}

#endif // BYTECODE_H__