#include "objects/vm.hh"
#include "heap.hh"

namespace runtime {

void VirtualMachine::Execute() {
    Handle this_handle = heap()->GetHandle(Primitive::Reference(this));
    setupGlobalEnvrionment();
}

void VirtualMachine::setupGlobalEnvrionment() {
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

} // namespace runtime