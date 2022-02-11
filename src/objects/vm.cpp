#include "objects/vm.hh"
#include "heap.hh"

namespace runtime {

void VirtualMachine::Execute() {
    Handle this_handle = heap()->GetHandle(Primitive::Reference(this));
    setup();
}

void VirtualMachine::setupSymbolTable() {
    Handle map1 = heap()->GetHandle(
        Primitive::Reference(heap()->NewMap())
    );

    Handle map2 = heap()->GetHandle(
        Primitive::Reference(heap()->NewMap())
    );

    symbol_table_slot() = Primitive::Reference(
        heap()->NewSymbolTable(
            map1.GetData(), map2.GetData()
        )
    );
}

void VirtualMachine::setupGlobalEnv() {

    Handle env = heap()->GetHandle(
        Primitive::Reference(heap()->NewMap())
    );

    global_env_slot() = Primitive::Reference(
        heap()->NewEnvironment(
            Primitive::Nil(),
            env.GetData()
        )
    );
}

Envrionment* VirtualMachine::env() {
    return global_env_slot().GetReference()->AsEnvrionment();
}

Heap* VirtualMachine::heap() {
    Heap* heap_ptr = static_cast<Heap*>(heap_slot().GetNativeReference());
    return heap_ptr;
}

SymbolTable* VirtualMachine::sym() {
    return symbol_table_slot().GetReference()->AsSymbolTable();
}

} // namespace runtime