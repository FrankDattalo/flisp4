#ifndef STACK_HH__
#define STACK_HH__

#include <memory>
#include <vector>

#include "bytecode.hh"
#include "object.hh"

class StackFrame {
private:
    std::unique_ptr<StackFrame> outer;
    std::vector<Object> locals;
    std::vector<Object> temps;
    std::shared_ptr<Function*> fn;
    std::size_t program_counter;
public:
};

#endif // STACK_HH__