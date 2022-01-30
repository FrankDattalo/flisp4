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
        std::uint64_t string_constant_count = ReadU64(input_file);
        DEBUGLN("Reading string constants");
        std::vector<std::string> string_constants;
        string_constants.reserve(string_constant_count);
        for (std::uint64_t i = 0; i < string_constant_count; i++) {
            string_constants.push_back(ReadString(input_file));
        }

        DEBUGLN("Done reading compiled file");

        return File{
            version,
            std::move(fns),
            std::move(string_constants)
        };
    }

private:
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
        std::uint64_t bytecode = ReadU64(stream);
        BytecodeType casted = static_cast<BytecodeType>(bytecode);

        if (Bytecode::HasArg(casted)) {
            switch (Bytecode::ArgType(casted)) {
            case BytecodeArgType::Signed: {
                std::int64_t i = ReadI64(stream);
                BytecodeArg arg{i};
                return Bytecode{casted, BytecodeArgType::Signed, arg};
            }
            case BytecodeArgType::Unsigned: {
                std::uint64_t u = ReadU64(stream);
                BytecodeArg arg{u};
                return Bytecode{casted, BytecodeArgType::Unsigned, arg};
            }
            default:
                std::string msg{"Unhandled bytecode arg type in ReadBytecode: "};
                msg.append(std::to_string(static_cast<std::uint64_t>(Bytecode::ArgType(casted))));
                throw std::runtime_error{msg};
            }
        } else {
            return Bytecode{casted};
        }
    }
};

static_assert(sizeof(char) == 1); // 1 byte

#endif // BYTECODE_READER_H__