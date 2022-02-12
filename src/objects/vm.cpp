#include "objects/vm.hh"
#include "heap.hh"

namespace runtime {

void VirtualMachine::Execute(VirtualMachine* self_) {
    ReferenceHandle<VirtualMachine> self = self_->heap()->GetHandle(self_);
    DEBUGLN("Heap root at " << self.GetPointer());
    setup(self.GetPointer());
}

void VirtualMachine::setupSymbolTable(VirtualMachine* self) {
    ReferenceHandle<Map> map1 = self->heap()->GetHandle(
        self->heap()->NewMap()
    );

    ReferenceHandle<Map> map2 = self->heap()->GetHandle(
        self->heap()->NewMap()
    );

    self->symbol_table_slot() = Primitive::Reference(
        self->heap()->NewSymbolTable(
            map1.GetData(), map2.GetData()
        )
    );
}

void VirtualMachine::setupGlobalEnv(VirtualMachine* self) {

    ReferenceHandle<Map> env = self->heap()->GetHandle(
        self->heap()->NewMap()
    );

    self->global_env_slot() = Primitive::Reference(
        self->heap()->NewEnvironment(
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