#ifndef STACK_HH__
#define STACK_HH__

#include <memory>
#include <vector>

#include "bytecode.hh"
#include "object.hh"
#include "memory_semantic_macros.hh"
#include "heap.hh"

class StackFrame {
private:
    std::unique_ptr<StackFrame> outer;
    std::vector<Object> locals;
    std::vector<Object> temps;
    const Function* fn;
    std::size_t program_counter;
public:
    StackFrame(std::unique_ptr<StackFrame> _outer, const Function* _fn)
    : outer{std::move(_outer)}, fn{_fn}, program_counter{0}
    {
        locals.resize(_fn->GetLocals());
    }

    ~StackFrame() = default;

    NOT_MOVEABLE(StackFrame);

    NOT_COPYABLE(StackFrame);

    // Once this method is called this stack frame no longer
    // has an outer stack frame
    std::unique_ptr<StackFrame> GetOuter() {
        if (!outer) {
            throw std::runtime_error{std::string{"GetOuter called on stack frame with no outer frame"}};
        }
        return std::move(outer);
    }

    void Mark(Heap* heap) {
        for (Object& local: locals) {
            heap->TransferIfReference(&local);
        }
        for (Object& temp: temps) {
            heap->TransferIfReference(&temp);
        }
    }

    StackFrame* GetNullableOuter() {
        return this->outer.get();
    }

    const Function* GetFunction() {
        return fn;
    }

    std::uint64_t LocalCount() {
        return locals.size();
    }

    Object GetLocal(std::uint64_t index) {
        return this->locals.at(index);
    }

    void SetLocal(std::uint64_t index, Object val) {
        this->locals.at(index) = val;
    }

    void Push(Object obj) {
        this->temps.push_back(obj);
    }

    Object OffsetFromTop(std::size_t offset) {
        std::size_t index = (this->temps.size() - 1) - offset;
        return this->temps.at(index);
    }

    std::uint64_t TempCount() {
        return temps.size();
    }

    Object GetTemp(std::uint64_t index) {
        return this->temps.at(index);
    }

    Object Pop() {
        Object result = this->temps.back();
        this->temps.pop_back();
        return result;
    }

    std::size_t GetProgramCounter() {
        return program_counter;
    }

    void SetProgramCounter(std::size_t value) {
        this->program_counter = value;
    }

    void AdvanceProgramCounter() {
        this->program_counter += 1;
    }
};

static_assert(sizeof(std::uint64_t) == sizeof(std::size_t));

#endif // STACK_HH__