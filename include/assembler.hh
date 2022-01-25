#ifndef ASSEMBLER_HH__
#define ASSEMBLER_HH__

#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>

#include "memory_semantic_macros.hh"
#include "bytecode.hh"
#include "debug.hh"
#include "bytecode_writer.hh"

class Assembler {
public:
    Assembler() = delete;

    ~Assembler() = delete;

    NOT_COPYABLE(Assembler);

    NOT_MOVEABLE(Assembler);

    static void Assemble(const std::string & input_path, const std::string & output_path) {

        DEBUGLN("Opening file");
        std::ifstream input_file;
        input_file.open(input_path, std::ios::in);

        if (!input_file.is_open()) {
            std::string msg{"Could not open file "};
            msg.append(input_path);
            throw std::runtime_error{msg};
        }

        DEBUGLN("File opened");

        // wip file
        std::uint64_t version;
        std::vector<Function> functions;
        std::vector<std::int64_t> integer_constants;
        std::vector<char> character_constants;
        std::vector<std::string> symbol_constants;

        // wip function
        std::uint64_t arity;
        std::uint64_t locals;
        std::vector<Bytecode> bytecode;

        std::string line;
        while (true) {
            DEBUGLN("Reading line");

            std::getline(input_file, line);

            if (input_file.eof()) {
                break;
            }

            DEBUGLN("LINE: '" << line << "'");

            if (!hasNonWhitespace(line)) {
                //DEBUGLN("Blank line, skipping...");
                continue;
            }
            //DEBUGLN("Has non whitespace " << hasNonWhitespace(line));

            std::vector<std::string> split = splitOnSpaces(line);

            const std::string& first = split.at(0);
            if (first.at(0) == ';') {
                // comment line
                //DEBUGLN("Comment, skipping...");
                continue;
            }

            if (first == "@version") {
                version = std::stoull(split.at(1));
                //DEBUGLN("Version = " << version);
            } else if (first == "@function") {
                arity = 0;
                locals = 0;
                bytecode.clear();
                //DEBUGLN("Begin function");
            } else if (first == "@endfunction") {
                //DEBUGLN("End function");
                functions.emplace_back(arity, locals, bytecode);
            } else if (first == "@int") {
                //DEBUGLN("Int constant");
                integer_constants.push_back(std::stoll(split.at(1)));
            } else if (first == "@char") {
                //DEBUGLN("Char constant");
                character_constants.push_back(split.at(1).at(0));
            } else if (first == "@symbol") {
                //DEBUGLN("Symbol constant");
                symbol_constants.push_back(split.at(1));
            } else if (first == "@arity") {
                arity = std::stoull(split.at(1));
                //DEBUGLN("Arity = " << arity);
            } else if (first == "@locals") {
                locals = std::stoull(split.at(1));
                //DEBUGLN("Locals = " << locals);
            } else /* it's a bytecode */ {
                Bytecode bc = getBytecode(split);
                bytecode.push_back(bc);
                //DEBUGLN("Bytecode = " << Bytecode::TypeToString(bc.GetType()));
            }
        }

        DEBUGLN("Writing out file");

        File file{
            std::move(version), 
            std::move(functions), 
            std::move(integer_constants), 
            std::move(character_constants), 
            std::move(symbol_constants)};

        BytecodeWriter::Write(file, output_path);

        DEBUGLN("Done!");
    }
private:
    static std::vector<std::string> splitOnSpaces(const std::string & str) {

        std::stringstream stream{str};

        std::vector<std::string> result;
        std::string word;

        while (stream >> word) {
            result.push_back(word);
        }

        return result;
    }

    static bool hasNonWhitespace(const std::string & str) {
        for (char c : str) {
            if (c != '\t' && c != ' ') {
                //DEBUGLN("Non whitespace char found " << static_cast<int>(c));
                return true;
            }
        }
        return false;
    }

    static Bytecode getBytecode(const std::vector<std::string>& line) {
        std::map<std::string, BytecodeType> bytecode_by_type{
            {"hlt", BytecodeType::Halt},
            {"ll", BytecodeType::LoadLocal},
            {"sl", BytecodeType::StoreLocal},
            {"jnz", BytecodeType::JumpIfFalse},
            {"jmp", BytecodeType::Jump},
            {"call", BytecodeType::Invoke},
            {"true", BytecodeType::LoadTrue},
            {"false", BytecodeType::LoadFalse},
            {"nil", BytecodeType::LoadNil},
            {"li", BytecodeType::LoadInteger},
            {"ls", BytecodeType::LoadSymbol},
            {"lc", BytecodeType::LoadCharacter},
            {"lf", BytecodeType::LoadField},
            {"sf", BytecodeType::StoreField},
            {"ret", BytecodeType::Return},
            {"fn", BytecodeType::MakeFunction}
        };

        const std::string & bc = line.at(0);

        DEBUGLN("First '" << bc << "'");

        if (bytecode_by_type.find(bc) == bytecode_by_type.end()) {
            std::string message{"Unknown bytecode: "};
            message.append(line.at(0));
            throw std::runtime_error{message};
        }

        BytecodeType type = bytecode_by_type.at(line.at(0));
        std::uint64_t arg = 0;

        if (Bytecode::HasArg(type)) {
            arg = std::stoull(line.at(1));
        }

        return Bytecode{type, arg};
    }
};

#endif // ASSEMBLER_HH__