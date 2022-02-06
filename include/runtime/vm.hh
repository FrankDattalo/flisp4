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
        stack.Push(getEntrypoint());
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
        frame->Get
        const std::string& str = this->file.GetConstants().at(bc.GetUnsignedArg()).GetStringConstant();
        Object* result = this->heap.AllocateString(str);
        Object tmp;
        tmp.SetReference(result);
        this->frame->Push(tmp);
        this->frame->AdvanceProgramCounter();
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

    void Invoke(const bytecode::Bytecode& bc) {
        // TODO
    }

    void JumpIfFalse(const bytecode::Bytecode& bc) {
        Object res = this->frame->Pop();
        if (res.IsType(ObjectType::Boolean) && !res.GetBoolean()) {
            this->frame->SetProgramCounter(bc.GetUnsignedArg());
        } else {
            this->frame->AdvanceProgramCounter();
        }
    }

    void Jump(const bytecode::Bytecode& bc) {
        this->frame->SetProgramCounter(bc.GetUnsignedArg());
    }

    void LoadTrue(const bytecode::Bytecode& bc) {
        Object temp;
        temp.SetBoolean(true);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadFalse(const bytecode::Bytecode& bc) {
        Object temp;
        temp.SetBoolean(false);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadNil(const bytecode::Bytecode& bc) {
        this->frame->Push(Object{});
        this->frame->AdvanceProgramCounter();
    }

    void LoadInteger(const bytecode::Bytecode& bc) {
        Object temp;
        temp.SetInteger(this->file.GetConstants().at(bc.GetUnsignedArg()).GetIntegerConstant());
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void Return(const bytecode::Bytecode& bc) {
        Object ret = this->frame->Pop();
        this->frame = this->frame->GetOuter();
        this->frame->Push(ret);
        this->frame->AdvanceProgramCounter();
    }

    void pushFrame(const Function* fn) {
        this->frame = std::make_unique<StackFrame>(std::move(this->frame), fn);
    }

    const bytecode::Function* getEntrypoint() {
        // TODO: separate this into some other validation class
        if (this->file.GetFunctions().size() == 0) {
            throw std::runtime_error{std::string{"No functions defined in file"}};
        }
        auto result = &this->file.GetFunctions().at(0);
        if (result->GetArity() != 0) {
            throw std::runtime_error{std::string{"Expected entrypoint function to have arity of 0"}};
        }

        auto result = files.LookupFunction("main", "main");
        if (result == std::nullopt) {
            throw std::runtime_error{std::string{"No main/main function defined"}};
        }


        return result;
    }

    void debugPrintFrame(StackFrame* frame) {
        std::cout << "FRAME -------------------------------------------\n";
        std::cout << "| ARITY    " << frame->GetFunction()->GetArity() << "\n";
        std::cout << "| LOCALS   [";
        for (std::size_t i = 0; i < frame->LocalCount(); i++) {
            if (i != 0) {
                std::cout << ", ";
            }
            Object local = frame->GetLocal(i);
            std::cout << local.ToDebugString();
        }
        std::cout << "]\n";
        std::cout << "| TEMPS    [";
        for (std::size_t i = 0; i < frame->TempCount(); i++) {
            if (i != 0) {
                std::cout << ", ";
            }
            Object temp = frame->GetTemp(i);
            std::cout << temp.ToDebugString();
        }
        std::cout << "]\n";
        std::cout << "| PC       " << frame->GetProgramCounter() << "\n";
        std::cout << "| BYTECODE \n";
        for (std::size_t i = 0; i < frame->GetFunction()->GetBytecode().size(); i++) {
            std::cout << "| [" << i << "] ";
            const bytecode::Bytecode& bc = frame->GetFunction()->GetBytecode().at(i);
            std::cout << bc.GetTypeToString() << " " << bc.ArgToString();
            if (i == frame->GetProgramCounter()) {
                std::cout << " <~~~~~~~~~~~~~~~~~~";
            }
            std::cout << "\n";
        }
        std::cout << "-------------------------------------------------\n";
    }

    void debugPrint() {
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        bool first = true;
        StackFrame* frame = this->frame.get();
        while (frame != nullptr) {
            if (!first) {
                std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
            }
            debugPrintFrame(frame);
            frame = frame->GetNullableOuter();
            first = false;
        }
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        std::cout << std::endl;
    }

    void waitForInput() {
        DEBUGLN("Press [Enter] to continue...");
        std::string line;
        std::getline(std::cin, line);
    }

    void registerNatives() {
        this->registry.Register(NativeFunction{"println", 1, NativePrintln});
    }

    static void NativePrintln(VirtualMachine* vm) {
        Object to_print = vm->frame->Pop();
        std::cout << to_print.ToDebugString() << std::endl; // TODO: change this
        Object obj;
        obj.SetNil();
        vm->frame->Push(obj);
    }
};

}

#endif // VM_H__