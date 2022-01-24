#ifndef BYTECODE_READER_H__
#define BYTECODE_READER_H__

#include <stdexcept>
#include <fstream>

#include "bytecode.hh"

class BytecodeReader {
public:
    BytecodeReader() = delete;

    ~BytecodeReader() = delete;

    NOT_COPYABLE(BytecodeReader);

    NOT_MOVEABLE(BytecodeReader);

    static File Read(const std::string & path) {
        std::ifstream input_file{path};
        
        // read functions
        std::uint64_t fn_count = read_u64(input_file);
        std::vector<Function> fns;
        fns.reserve(fn_count);
        for (std::uint64_t i = 0; i < fn_count; i++) {
            fns.push_back(read_function(input_file));
        }

        // read int constants
        std::uint64_t int_constant_count = read_u64(input_file);
        std::vector<std::int64_t> int_constants;
        int_constants.reserve(int_constant_count);
        for (std::uint64_t i = 0; i < int_constant_count; i++) {
            int_constants.push_back(read_i64(input_file));
        }

        // read char constants
        std::uint64_t char_constant_count = read_u64(input_file);
        std::vector<char> char_constants;
        char_constants.reserve(char_constant_count);
        for (std::uint64_t i = 0; i < char_constant_count; i++) {
            char_constants.push_back(read_char(input_file));
        }

        // read symbol constants
        std::uint64_t symbol_constant_count = read_u64(input_file);
        std::vector<std::string> symbol_constants;
        symbol_constants.reserve(symbol_constant_count);
        for (std::uint64_t i = 0; i < symbol_constant_count; i++) {
            symbol_constants.push_back(read_string(input_file));
        }

        return File{
            std::move(fns),
            std::move(int_constants),
            std::move(char_constants),
            std::move(symbol_constants)
        };
    }
private:
    static std::string read_string(std::ifstream& stream) {
        std::uint64_t length = read_u64(stream);
        std::string result;
        for (std::uint64_t i = 0; i < length; i++) {
            result.push_back(read_char(stream));
        }
        return result;
    }

    static char read_char(std::ifstream& stream) {
        char data = '\0';
        stream >> data;
        return data;
    }

    static std::uint64_t read_u64(std::ifstream& stream) {
        std::uint64_t data = 0;
        stream >> data;
        return data;
    }

    static std::int64_t read_i64(std::ifstream& stream) {
        std::int64_t data = 0;
        stream >> data;
        return data;
    }

    static Function read_function(std::ifstream& stream) {
        std::uint64_t arity = read_u64(stream);
        std::uint64_t bytecode_length = read_u64(stream);
        std::vector<Bytecode> bytecode;
        bytecode.reserve(bytecode_length);
        for (std::uint64_t i = 0; i < bytecode_length; i++) {
            bytecode.push_back(read_bytecode(stream));
        }
        return Function{arity, std::move(bytecode)};
    }

    static Bytecode read_bytecode(std::ifstream& stream) {
        std::uint64_t bytecode = read_u64(stream);
        std::uint64_t arg = read_u64(stream);
        BytecodeType casted = static_cast<BytecodeType>(bytecode);
        return Bytecode{casted, arg};
    }
};

#endif // BYTECODE_READER_H__