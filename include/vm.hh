#ifndef OBJECTS_VM_HH__
#define OBJECTS_VM_HH__

#include "lib.hh"
#include "util.hh"
#include "objects.hh"
#include "heap.hh"
#include "symbol_table.hh"

#define PER_OPCODE(V) \
    V(load) \
    V(define) \
    V(set) \
    V(invoke) \
    V(lambda) \
    V(literal) \
    V(pop) \
    V(invoketail) \
    V(jumpiffalse) \
    V(jump) \
    V(return)

class VirtualMachine {
    Heap heap;
    SymbolTable symbol_table;
    Handle global_env;
    #define DEFINE_SYMBOL_FOR_OPCODE(V) Primitive symbol_##V;
    PER_OPCODE(DEFINE_SYMBOL_FOR_OPCODE)
    #undef DEFINE_SYMBOL_FOR_OPCODE
public:
    VirtualMachine() : heap{1000} {
        internSymbols();
    }

    ~VirtualMachine() = default;

    NOT_COPYABLE(VirtualMachine);

    NOT_MOVEABLE(VirtualMachine);

    void Execute(Handle frame) {
        while (keepGoing(frame)) {
            Handle bc = nextBytecode(frame);
            frame = dispatch(frame, bc);
        }
    }

private:
    Handle dispatch(Handle frame, Handle bc) {

        Symbol op = *bc.AsPair()->First().AsSymbol();

        #define DISPATCHER(opcode) \
            if (shallowEquals(op, symbol_##opcode)) { \
                return on_##opcode(frame, bc); \
            }
        PER_OPCODE(DISPATCHER)
        #undef DISPATCHER

        std::stringstream stream;
        stream << "Unknown bytecode: " << symbol_table.ToString(op);
        throw std::runtime_error{stream.str()};
    }

    Handle on_load(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_define(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_set(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_invoke(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_invoketail(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_lambda(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_literal(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_pop(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_jumpiffalse(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_jump(Handle frame, Handle bc) {
        return frame;
    }

    Handle on_return(Handle frame, Handle bc) {
        return frame;
    }

    bool keepGoing(Handle frame) const {
        Frame* f = frame.AsFrame();
        Integer pc = *f->ProgramCounter().AsInteger();
        Integer bc_length = f->BytecodeLength();
        return pc.Value() < bc_length.Value();
    }

    Handle nextBytecode(Handle frame) {
        Frame* f = frame.AsFrame();
        return heap.GetHandle(f->NextBytecode());
    }

    void internSymbols() {
        #define INTERN(s) symbol_##s = symbol_table.Intern(#s);
        PER_OPCODE(INTERN)
        #undef INTERN
    }

    bool shallowEquals(Primitive p1, Primitive p2) {
        return false;
    }
};

#endif // OBJECTS_VM_HH__