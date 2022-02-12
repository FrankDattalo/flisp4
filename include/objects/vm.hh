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

    static void Execute(VirtualMachine* vm);

private:
    static void setup(VirtualMachine* vm) {
        setupSymbolTable(vm);
        setupGlobalEnv(vm);
    }

    static void setupGlobalEnv(VirtualMachine* vm);
    static void setupSymbolTable(VirtualMachine* vm);

    Heap* heap();

    Envrionment* env();

    SymbolTable* sym();
};

};

#endif // OBJECTS_VM_HH__