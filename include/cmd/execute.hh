#ifndef EXECUTE_HH__
#define EXECUTE_HH__

#include <vector>
#include <string>

#include "bytecode/bytecode.hh"
#include "bytecode/bytecode_reader.hh"
#include "runtime/vm.hh"

namespace cmd {

void execute(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        throw std::runtime_error{std::string{"Bad format, expected: <program_name> execute <file_path>"}};
    }
    const std::string & file_name = args.at(2);
    bytecode::File result = bytecode::BytecodeReader::Read(file_name);
    runtime::VirtualMachine vm;
    vm.Run();
}

}

#endif // EXECUTE_HH__