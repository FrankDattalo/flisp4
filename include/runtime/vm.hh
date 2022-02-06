#ifndef VM_HH__
#define VM_HH__

#include <optional>

#include "bytecode/bytecode.hh"
#include "util/debug.hh"
#include "util/memory_semantic_macros.hh"
#include "heap.hh"
#include "object.hh"
#include "stack.hh"
#include "symbol_table.hh"
#include "nativeregistry.hh"
#include "fileregistry.hh"

namespace runtime {

class VirtualMachine {
private:
    FileRegistry files;
    NativeFunctionRegistry natives;
    CallStack stack;
    Heap heap;
public:
    VirtualMachine() : heap{1000} /* TODO change this */ {
        registerNatives();
    }

    virtual ~VirtualMachine() = default;

    NOT_COPYABLE(VirtualMachine);
    NOT_MOVEABLE(VirtualMachine);

    void Run() {
        DEBUGLN("Beginning vm");
        DEBUGLN("Adding root marker");
        heap.AddRootMarker(&stack);
        DEBUGLN("Adding initial frame");
        loadEntrypoint();
        DEBUGLN("Entering main loop");
        loop();
        DEBUGLN("Main loop terminated");
    }

    void RegisterNativeFunction(NativeFunction fn) {
        natives.Register(std::move(fn));
    }

private:
    void loop() {
        if (IS_DEBUG_ENABLED()) {
            DEBUGLN("VM START");
            debugPrint();
            waitForInput();
        }

        while (true) {
            StackFrame* frame = stack.CurrentFrame();
            std::size_t pc = frame->GetProgramCounter();

            DEBUGLN("PC: " << pc);

            const bytecode::Bytecode& bc = frame->GetBytecode(pc);

            DEBUGLN("Loaded bytecode: " << bc.GetTypeToString());

            bool terminate = applyBytecode(frame, bc);
            if (terminate) {
                break;
            }

            if (IS_DEBUG_ENABLED()) {
                DEBUGLN("VM AFTER INSTRUCTION");
                debugPrint();
                waitForInput();
            }
        }
    }

    bool applyBytecode(StackFrame* frame, const bytecode::Bytecode& bc) {
        bool terminate = false;

        struct Visitor : public bytecode::BytecodeVisitor {
            VirtualMachine* self;
            StackFrame* frame;
            bool& terminate;

            Visitor(VirtualMachine* _self, StackFrame* _frame, bool& _terminate)
            : self{_self}, frame{_frame}, terminate{_terminate}
            {}

            void OnJumpIfFalse(const bytecode::Bytecode& bc) override { DEBUGLN("JumpIfFalse"); self->JumpIfFalse(frame, bc); }
            void OnJump(const bytecode::Bytecode& bc) override { DEBUGLN("Jump"); self->Jump(frame, bc); }
            void OnLoadNil(const bytecode::Bytecode& bc) override { DEBUGLN("LoadNil"); self->LoadNil(frame, bc); }
            void OnReturn(const bytecode::Bytecode& bc) override { DEBUGLN("Return"); self->Return(frame, bc); }
            void OnLoadLocal(const bytecode::Bytecode& bc) override { DEBUGLN("LoadLocal"); self->LoadLocal(frame, bc); }
            void OnStoreLocal(const bytecode::Bytecode& bc) override { DEBUGLN("StoreLocal"); self->StoreLocal(frame, bc); }
            void OnLoadInteger(const bytecode::Bytecode& bc) override { DEBUGLN("LoadInteger"); self->LoadInteger(frame, bc); }
            void OnLoadString(const bytecode::Bytecode& bc) override { DEBUGLN("LoadString"); self->LoadString(frame, bc); }
            void OnLoadTrue(const bytecode::Bytecode& bc) override { DEBUGLN("LoadTrue"); self->LoadTrue(frame, bc); }
            void OnLoadFalse(const bytecode::Bytecode& bc) override { DEBUGLN("LoadFalse"); self->LoadFalse(frame, bc); }
            void OnInvoke(const bytecode::Bytecode& bc) override { DEBUGLN("Invoke"); self->Invoke(frame, bc); }
            void OnPop(const bytecode::Bytecode& bc) override { DEBUGLN("Pop"); self->Pop(frame, bc); }
            void OnHalt(const bytecode::Bytecode& bc) override { DEBUGLN("Halt"); terminate = true; }

        } visitor(this, frame, terminate);

        bc.Visit(visitor);

        return terminate;
    }

    void LoadLocal(StackFrame* frame, const bytecode::Bytecode& bc) {
        frame->Push(frame->GetLocal(bc.GetUnsignedArg()));
        frame->AdvanceProgramCounter();
    }

    void StoreLocal(StackFrame* frame, const bytecode::Bytecode& bc) {
        frame->SetLocal(bc.GetUnsignedArg(), frame->Pop());
        frame->AdvanceProgramCounter();
    }

    void Pop(StackFrame* frame, const bytecode::Bytecode& bc) {
        frame->Pop();
        frame->AdvanceProgramCounter();
    }

    void LoadString(StackFrame* frame, const bytecode::Bytecode& bc) {
        const std::string& str = frame->GetConstant(bc.GetUnsignedArg()).GetStringConstant();
        Object* result = this->heap.AllocateString(str);
        Object tmp;
        tmp.SetReference(result);
        frame->Push(tmp);
        frame->AdvanceProgramCounter();
    }

    // void InvokeNative(const bytecode::Bytecode& bc) {
    //     std::uint64_t num_args = this->frame->Pop().GetUnsignedInteger();
    //     const std::string& native_fn_name = this->file.GetConstants().at(bc.GetUnsignedArg()).GetStringConstant();
    //     const NativeFunction& fn = this->registry.Get(native_fn_name);
    //     if (fn.GetArity() != num_args) {
    //         // TODO, don't crash the vm
    //         throw std::runtime_error{std::string{"Arity mismatch on native function call"}};
    //     }
    //     fn.Apply(this);
    //     this->frame->AdvanceProgramCounter();
    // }

    // void InvokeFunction(const bytecode::Bytecode& bc) {
    //     const Function& fn = this->file.GetFunctions().at(bc.GetUnsignedArg());
    //     std::uint64_t num_args = this->frame->Pop().GetUnsignedInteger();
    //     if (fn.GetArity() != num_args) {
    //         // TODO, don't crash the vm
    //         throw std::runtime_error{std::string{"Arity mismatch on function call"}};
    //     }
    //     pushFrame(&fn);
    //     // assign the arguments from the outer stack frame
    //     StackFrame* outer = this->frame->GetNullableOuter();
    //     std::size_t local_index = num_args - 1;
    //     for (std::uint64_t i = 0; i < num_args; i++)  {
    //         this->frame->SetLocal(local_index, outer->Pop());
    //         local_index--;
    //     }
    // }

    void Invoke(StackFrame* frame, const bytecode::Bytecode& bc) {
        // TODO
    }

    void JumpIfFalse(StackFrame* frame, const bytecode::Bytecode& bc) {
        Object res = frame->Pop();
        if (res.IsType(ObjectType::Boolean) && !res.GetBoolean()) {
            frame->SetProgramCounter(bc.GetUnsignedArg());
        } else {
            frame->AdvanceProgramCounter();
        }
    }

    void Jump(StackFrame* frame, const bytecode::Bytecode& bc) {
        frame->SetProgramCounter(bc.GetUnsignedArg());
    }

    void LoadTrue(StackFrame* frame, const bytecode::Bytecode& bc) {
        Object temp;
        temp.SetBoolean(true);
        frame->Push(temp);
        frame->AdvanceProgramCounter();
    }

    void LoadFalse(StackFrame* frame, const bytecode::Bytecode& bc) {
        Object temp;
        temp.SetBoolean(false);
        frame->Push(temp);
        frame->AdvanceProgramCounter();
    }

    void LoadNil(StackFrame* frame, const bytecode::Bytecode& bc) {
        frame->Push(Object{});
        frame->AdvanceProgramCounter();
    }

    void LoadInteger(StackFrame* frame, const bytecode::Bytecode& bc) {
        std::int64_t value = frame->GetConstant(bc.GetUnsignedArg()).GetIntegerConstant();
        Object temp;
        temp.SetInteger(value);
        frame->Push(temp);
        frame->AdvanceProgramCounter();
    }

    void Return(StackFrame* frame, const bytecode::Bytecode& bc) {
        Object ret = frame->Pop();
        stack.Pop();
        frame = stack.CurrentFrame();
        frame->Push(ret);
        frame->AdvanceProgramCounter();
    }

    void pushFrame(const bytecode::Function* fn, const bytecode::File* file) {
        stack.Push(fn, file);
    }

    void loadEntrypoint() {
        // TODO: separate this into some other validation class?
        auto result = files.LookupFunction("main", "main");
        if (result == std::nullopt) {
            throw std::runtime_error{std::string{"No main/main function defined"}};
        }
        const bytecode::Function* fn = *result;
        const bytecode::File* file = *files.Lookup("main");
        pushFrame(fn, file);
    }

    void waitForInput() {
        DEBUGLN("Press [Enter] to continue...");
        std::string line;
        std::getline(std::cin, line);
    }

    void debugPrint() {
        std::cout << "Call Stack:" << std::endl;
        stack.DebugPrint();
    }

    void registerNatives() {
        natives.Register(NativeFunction{"println", 1, NativePrintln});
    }

    static void NativePrintln(runtime::VirtualMachine* vm) {
        Object to_print = vm->stack.CurrentFrame()->Pop();
        std::cout << to_print.ToDebugString() << std::endl; // TODO: change this
        Object obj;
        obj.SetNil();
        vm->stack.CurrentFrame()->Push(obj);
    }
};

}

#endif // VM_H__