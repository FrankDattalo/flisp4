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
#include "bytecode_reader.hh"
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
        std::string module_name;
        std::vector<std::string> import_names;
        std::vector<std::string> export_names;
        std::vector<Function> functions;
        std::vector<Constant> constants;

        // wip function
        std::string function_name;
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
                function_name = split.at(1);
                //DEBUGLN("Begin function");
            } else if (first == "@endfunction") {
                //DEBUGLN("End function");
                functions.emplace_back(function_name, arity, locals, bytecode);
            } else if (first == "@arity") {
                arity = std::stoull(split.at(1));
                //DEBUGLN("Arity = " << arity);
            } else if (first == "@locals") {
                locals = std::stoull(split.at(1));
                //DEBUGLN("Locals = " << locals);
            } else if (first == "@string") {
                std::stringstream line_stream{line};
                line_stream.ignore(std::string{"@string"}.size()); // skip over @string directive
                DEBUGLN("Reading string length");
                std::uint64_t length = 0;
                line_stream >> length;
                line_stream.ignore(1); // the space after the length
                DEBUGLN("String length: " << length);
                std::string str;
                str.reserve(length);
                for (std::uint64_t i = 0; i < length; i++) {
                    char c;
                    line_stream.read(&c, sizeof(char));
                    str.push_back(c);
                }
                DEBUGLN("Final string from '" << line << "' is '" << str << "'");
                constants.push_back(Constant(StringConstant{std::move(str)}));
            } else if (first == "@integer") {
                std::int64_t value = std::stoll(split.at(1));
                constants.push_back(Constant(IntegerConstant{value}));
            } else if (first == "@module") {
                module_name = split.at(1);
            } else if (first == "@import") {
                import_names.push_back(split.at(1));
            } else if (first == "@export") {
                export_names.push_back(split.at(1));
            } else if (first == "@invocation") {
                std::uint64_t module_name_index = std::stoll(split.at(1));
                std::uint64_t function_name_index = std::stoll(split.at(2));
                std::uint64_t argument_count = std::stoll(split.at(3));
                constants.push_back(Constant(InvocationConstant{Invocation{
                    module_name_index,
                    function_name_index,
                    argument_count
                }}));
            } else /* it's a bytecode */ {
                Bytecode bc = getBytecode(split);
                bytecode.push_back(bc);
                //DEBUGLN("Bytecode = " << Bytecode::TypeToString(bc.GetType()));
            }
        }

        DEBUGLN("Writing out file");

        File file{
            version, 
            std::move(module_name),
            std::move(import_names),
            std::move(export_names),
            std::move(functions), 
            std::move(constants)};

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

        const std::string & bc = line.at(0);

        DEBUGLN("First '" << bc << "'");

        BytecodeType type = Bytecode::TypeFromString(bc);
        BytecodeArg arg;

        struct Visitor : BytecodeArgTypeVisitor {
            BytecodeArg& result;
            const std::vector<std::string>& line;

            Visitor(BytecodeArg& _result, const std::vector<std::string>& _line): result{_result}, line{_line} {}

            void OnNone(BytecodeArgType) override {
                // already initialize to none
            }

            void OnUnsigned(BytecodeArgType) override {
                std::uint64_t u = std::stoull(line.at(1));
                BytecodeArg arg{u};
                result = arg;
            }

        } visitor (arg, line);

        Bytecode::VisitArgType(type, visitor);

        Bytecode result{type, arg};

        return result;
    }
};

#endif // ASSEMBLER_HH__