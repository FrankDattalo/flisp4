#ifndef BYTECODE_READER_H__
#define BYTECODE_READER_H__

#include <istream>
#include <iostream>

#include "bytecode.hh"

class BytecodeReader {
private:
    std::istream input;
public:
    BytecodeReader(std::istream _input) {
        input = _input;
    }

    ~BytecodeReader() = default;

    NOT_COPYABLE(BytecodeReader);

    NOT_MOVEABLE(BytecodeReader);


};

#endif // BYTECODE_READER_H__