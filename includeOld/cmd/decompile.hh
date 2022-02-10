#ifndef DECOMPILE_HH__
#define DECOMPILE_HH__

#include <string>
#include <vector>

#include "bytecode/bytecode.hh"
#include "bytecode/bytecode_reader.hh"

namespace cmd {

void decompile(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        throw std::runtime_error{std::string{"Bad format, expected: <program_name> decompile <file_path>"}};
    }
    const std::string & file_name = args.at(2);
    bytecode::File result = bytecode::BytecodeReader::Read(file_name);
    std::cout << "Module: " << result.GetModuleName() << "\n";
    std::cout << "Imports:\n";
    for (std::size_t i = 0; i < result.GetImportNames().size(); i++) {
        std::cout << "- Import[" << i << "] = " << result.GetImportNames().at(i) << "\n";
    }
    std::cout << "Exports:\n";
    for (std::size_t i = 0; i < result.GetExportNames().size(); i++) {
        std::cout << "- Exports[" << i << "] = " << result.GetExportNames().at(i) << "\n";
    }
    std::cout << "Functions:\n";
    for (std::size_t i = 0; i < result.GetFunctions().size(); i++) {
        std::cout << "- Fn[" << i << "]\n";
        std::cout << "  - Name: " << result.GetFunctions().at(i).GetName() << std::endl;
        std::cout << "  - Arity: " << result.GetFunctions().at(i).GetArity() << std::endl;
        std::cout << "  - Locals: " << result.GetFunctions().at(i).GetLocals() << std::endl;
        std::cout << "  - Bytecode:\n";
        for (std::size_t j = 0; j < result.GetFunctions().at(i).GetBytecode().size(); j++) {
            const bytecode::Bytecode & bc = result.GetFunctions().at(i).GetBytecode().at(j);
            std::cout 
                << "    - [" << j << "] " << bc.GetTypeToString() << " " << bc.ArgToString() << std::endl;
        }
    }
    std::cout << "Constants:\n";
    for (std::size_t i = 0; i < result.GetConstants().size(); i++) {
        std::cout << "- String[" << i << "] = " << result.GetConstants().at(i).ToString() << "\n";
    }
}

}

#endif // DECOMPILE_HH__