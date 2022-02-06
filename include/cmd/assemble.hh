#ifndef ASSEMBLE_HH__
#define ASSEMBLE_HH__

#include <vector>
#include <string>

#include "bytecode/assembler.hh"

namespace cmd {

void assemble(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        throw std::runtime_error{std::string{"Bad format, expected: <program_name> assemble <input_file_path> <output_file_path>"}};
    }
    const std::string & input_file = args.at(2);
    const std::string & output_file = args.at(3);
    bytecode::Assembler::Assemble(input_file, output_file);
}

}

#endif