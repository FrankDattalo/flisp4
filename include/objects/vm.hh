#ifndef OBJECTS_VM_HH__
#define OBJECTS_VM_HH__

#include "lib/std.hh"
#include "structure.hh"

namespace runtime {

class VirtualMachine : public Structure<Object::Type::VirtualMachine, 3> {
private:
    FIELD(0, heap_slot);
    FIELD(1, global_env_slot);
    FIELD(2, symbol_table_slot);
public:
    VirtualMachine(Primitive _heap) : Structure() {
        heap_slot() = _heap;
        global_env_slot() = Primitive::Nil();
        symbol_table_slot() = Primitive::Nil();
    }

    void Execute();

private:
    void setup() {
        setupSymbolTable();
        setupGlobalEnv();
    }

    void setupGlobalEnv();
    void setupSymbolTable();

    Heap* heap();

    Envrionment* env();

    SymbolTable* sym();
};

};

#endif // OBJECTS_VM_HH__