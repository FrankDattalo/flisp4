#ifndef BYTECODE_READER_H__
#define BYTECODE_READER_H__

#include <stdexcept>
#include <fstream>

#include "memory_semantic_macros.hh"
#include "bytecode.hh"
#include "debug.hh"

class BytecodeReader {
private:
    static const std::uint64_t COMPATIBLE_VERSION = 1;
public:
    BytecodeReader() = delete;

    ~BytecodeReader() = delete;

    NOT_COPYABLE(BytecodeReader);

    NOT_MOVEABLE(BytecodeReader);

    static File Read(const std::string & path) {
        std::ifstream input_file;
        input_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        input_file.open(path, std::ios::binary|std::ios::in);

        std::uint64_t version = ReadU64(input_file);

        if (version != COMPATIBLE_VERSION) {
            std::string msg{"Incompatible bytecode version: "};
            msg.append(std::to_string(version));
            msg.append(" need ");
            msg.append(std::to_string(COMPATIBLE_VERSION));
            throw std::runtime_error{msg};
        }

        // read functions
        DEBUGLN("Reading functions");
        std::uint64_t fn_count = ReadU64(input_file);
        std::vector<Function> fns;
        fns.reserve(fn_count);
        for (std::uint64_t i = 0; i < fn_count; i++) {
            fns.push_back(ReadFunction(input_file));
        }

        // read string constants
        std::uint64_t constant_count = ReadU64(input_file);
        DEBUGLN("Reading constants");
        std::vector<Constant> constants;
        constants.reserve(constant_count);
        for (std::uint64_t i = 0; i < constant_count; i++) {
            constants.push_back(ReadConstant(input_file));
        }

        DEBUGLN("Done reading compiled file");

        return File{
            version,
            std::move(fns),
            std::move(constants)
        };
    }

private:
    static Constant ReadConstant(std::ifstream& stream) {
        std::uint8_t constantTypeByte = ReadU8(stream);
        ConstantType constantType = static_cast<ConstantType>(constantTypeByte);

        Constant constant(IntegerConstant{0}); // default initialize with some value

        struct Visitor : public ConstantTypeVisitor {
            Constant& constant;
            std::ifstream& stream;

            Visitor(Constant& _constant, std::ifstream& _stream)
            : constant{_constant}, stream{_stream}
            {}

            void OnInteger(ConstantType) override {
                constant = Constant(IntegerConstant{ReadI64(stream)});
            }

            void OnString(ConstantType) override {
                constant = Constant(StringConstant{ReadString(stream)});
            }

        } visitor(constant, stream);

        Constant::VisitType(constantType, visitor);

        return constant;
    }

    static std::string ReadString(std::ifstream& stream) {
        std::uint64_t length = ReadU64(stream);
        //DEBUGLN("Reading string of " << length << " characters");
        std::string result;
        for (std::uint64_t i = 0; i < length; i++) {
            result.push_back(ReadChar(stream));
        }
        return result;
    }

    static char ReadChar(std::ifstream& stream) {
        char data = '\0';
        stream.read(reinterpret_cast<char*>(&data), sizeof(data));
        return data;
    }

    static char ReadU8(std::ifstream& stream) {
        std::uint8_t data = 0;
        stream.read(reinterpret_cast<char*>(&data), sizeof(data));
        return data;
    }

    static std::uint64_t ReadU64(std::ifstream& stream) {
        std::uint64_t data = 0;
        stream.read(reinterpret_cast<char*>(&data), sizeof(data));
        return data;
    }

    static std::int64_t ReadI64(std::ifstream& stream) {
        std::int64_t data = 0;
        stream.read(reinterpret_cast<char*>(&data), sizeof(data));
        return data;
    }

    static Function ReadFunction(std::ifstream& stream) {
        std::uint64_t arity = ReadU64(stream);
        std::uint64_t locals = ReadU64(stream);
        std::uint64_t bytecode_length = ReadU64(stream);
        std::vector<Bytecode> bytecode;
        bytecode.reserve(bytecode_length);
        for (std::uint64_t i = 0; i < bytecode_length; i++) {
            bytecode.push_back(ReadBytecode(stream));
        }
        return Function{arity, locals, std::move(bytecode)};
    }

    static Bytecode ReadBytecode(std::ifstream& stream) {
        std::uint8_t bytecode = ReadU8(stream);

        BytecodeType type = static_cast<BytecodeType>(bytecode);
        BytecodeArg arg;

        struct Visitor : public BytecodeArgTypeVisitor {
            BytecodeArg& arg;
            std::ifstream& stream;

            Visitor(BytecodeArg& _arg, std::ifstream& _stream)
            : arg{_arg}, stream{_stream}
            {}

            void OnUnsigned(BytecodeArgType) override {
                std::uint64_t u = ReadU64(stream);
                BytecodeArg result{u};
                arg = result;
            }

            void OnNone(BytecodeArgType) override {
                // intentionally empty
            }

        } visitor(arg, stream);

        Bytecode::VisitArgType(type, visitor);

        return Bytecode{type, arg};
    }
};

static_assert(sizeof(char) == 1); // 1 byte

#endif // BYTECODE_READER_H__