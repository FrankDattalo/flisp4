#ifndef STACK_HH__
#define STACK_HH__

#include <memory>
#include <vector>

#include "stack.hh"
#include "bytecode/bytecode.hh"
#include "object.hh"
#include "util/memory_semantic_macros.hh"
#include "heap.hh"

namespace runtime {

class StackFrame {
private:
    std::vector<Object> locals;
    std::vector<Object> temps;
    const bytecode::Function* fn;
    const bytecode::File* file;
    std::size_t program_counter;
public:
    StackFrame() {
        fn = nullptr;
        file = nullptr;
        program_counter = 0;
    }

    NOT_MOVEABLE(StackFrame);

    NOT_COPYABLE(StackFrame);

    void Initialize(const bytecode::Function* _fn, const bytecode::File* _file) {
        locals.resize(_fn->GetLocals());
        temps.clear();
        fn = _fn;
        file = _file;
        program_counter = 0;
    }

    void Push(Object obj) {
        temps.push_back(obj);
    }

    Object Pop() {
        Object result = temps.back();
        temps.pop_back();
        return result;
    }

    const bytecode::Bytecode& GetBytecode(std::size_t index) const {
        return fn->GetBytecode().at(index);
    }

    const bytecode::Constant& GetConstant(std::size_t index) const {
        return file->GetConstants().at(index);
    }

    std::uint64_t LocalCount() const {
        return locals.size();
    }

    Object GetLocal(std::uint64_t index) const {
        return this->locals.at(index);
    }

    void SetLocal(std::uint64_t index, Object val) {
        this->locals.at(index) = val;
    }

    std::size_t GetProgramCounter() const {
        return program_counter;
    }

    void SetProgramCounter(std::size_t value) {
        this->program_counter = value;
    }

    void AdvanceProgramCounter() {
        this->program_counter += 1;
    }
};

class CallStack : public RootMarker {
private:
    constexpr static std::size_t STACK_SIZE = 4096;
    std::array<StackFrame, STACK_SIZE> frames;
    std::size_t first_free;
public:
    CallStack() {
        first_free = 0;
    }

    virtual ~CallStack() = default;

    void Pop() {
        if (first_free == 0) {
            throw std::runtime_error{"Pop on empty call stack"};
        }
        first_free -= 1;
    }

    void Push(const bytecode::Function* fn, const bytecode::File* file) {
        if (first_free == frames.size()) {
            throw std::runtime_error{"Push on full call stack"};
        }
        frames.at(first_free).Initialize(fn, file);
        first_free += 1;
    }

    StackFrame* CurrentFrame() {
        if (first_free == 0) {
            throw std::runtime_error{"No active stack frame"};
        }
        return &frames.at(first_free - 1);
    }

    void Mark(Heap* heap) override {
        // TODO
        // for (Object& local: locals) {
        //     heap->TransferIfReference(&local);
        // }
        // for (Object& temp: temps) {
        //     heap->TransferIfReference(&temp);
        // }
    }
};

static_assert(sizeof(std::uint64_t) == sizeof(std::size_t));

}

#endif // STACK_HH__