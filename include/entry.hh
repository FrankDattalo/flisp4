#ifndef ENTRY_HH__
#define ENTRY_HH__

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <system_error>

#include "assembler.hh"
#include "bytecode_reader.hh"
#include "bytecode_writer.hh"
#include "bytecode.hh"
#include "debug.hh"
#include "heap.hh"
#include "object.hh"
#include "stack.hh"
#include "symbol_table.hh"
#include "vm.hh"

std::vector<std::string> getArgs(int argc, char** argv) {
    std::vector<std::string> result;
    result.reserve(argc);
    for (int i = 0; i < argc; i++) {
        result.emplace_back(argv[i], strlen(argv[i]));
    }
    return result;
}

using CommandFunction = void(*)(const std::vector<std::string>&);

void decompile(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        throw std::runtime_error{std::string{"Bad format, expected: <program_name> decompile <file_path>"}};
    }
    const std::string & file_name = args.at(2);
    File result = BytecodeReader::Read(file_name);
    std::cout << "Functions:\n";
    for (std::size_t i = 0; i < result.GetFunctions().size(); i++) {
        std::cout << "- Fn[" << i << "]\n";
        std::cout << "  - Arity: " << result.GetFunctions().at(i).GetArity() << std::endl;
        std::cout << "  - Locals: " << result.GetFunctions().at(i).GetLocals() << std::endl;
        std::cout << "  - Bytecode:\n";
        for (std::size_t j = 0; j < result.GetFunctions().at(i).GetBytecode().size(); j++) {
            const Bytecode & bc = result.GetFunctions().at(i).GetBytecode().at(j);
            std::cout 
                << "    - [" << j << "] " << Bytecode::TypeToString(bc.GetType()) <<
                " " << bc.ArgToString() << std::endl;
        }
    }
    std::cout << "Strings:\n";
    for (std::size_t i = 0; i < result.GetStringConstants().size(); i++) {
        std::cout << "- String[" << i << "] = " << result.GetStringConstants().at(i) << "\n";
    }
}

void execute(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        throw std::runtime_error{std::string{"Bad format, expected: <program_name> execute <file_path>"}};
    }
    const std::string & file_name = args.at(2);
    File result = BytecodeReader::Read(file_name);
    std::uint64_t heap_size = 1000; // TODO: change this in the future
    VirtualMachine vm{std::move(result), heap_size};
    vm.Run();
}

void assemble(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        throw std::runtime_error{std::string{"Bad format, expected: <program_name> assemble <input_file_path> <output_file_path>"}};
    }
    const std::string & input_file = args.at(2);
    const std::string & output_file = args.at(3);
    Assembler::Assemble(input_file, output_file);
}

CommandFunction selectCommandEntry(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw std::runtime_error{std::string{"No arguments given, at least one is expected"}};
    }
    const std::string& subcommand = args.at(1);
    if (subcommand == "decompile") {
        return decompile;
    }
    if (subcommand == "execute") {
        return execute;
    }
    if (subcommand == "assemble") {
        return assemble;
    }
    std::string message{"Unknown command: "};
    message.append(subcommand);
    throw std::runtime_error{message};
}

void printArgs(const std::vector<std::string>& args) {
    DEBUGLN("Arguments:");
    for (std::size_t i = 0; i < args.size(); i++) {
        DEBUGLN("[" << i << "] " << args.at(i));
    }
}

int entry(int argc, char** argv) {
    try {
        std::vector<std::string> args = getArgs(argc, argv);
        if (IS_DEBUG_ENABLED()) {
            printArgs(args);
        }
        CommandFunction fn = selectCommandEntry(args);
        fn(args);
        return 0;
    } catch (const std::system_error& e) {
        std::cerr << "System error: " << e.code() << " " << e.what() << std::endl;
        return 1;
    } catch (const std::exception & e) {
        std::cerr << "Uncaught error: " << e.what() << std::endl;
        return 1;
    }
}

#endif // ENTRY_H__