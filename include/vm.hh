#ifndef VM_HH__
#define VM_HH__

#include <memory>

#include "bytecode.hh"
#include "bytecode_reader.hh"
#include "debug.hh"
#include "heap.hh"
#include "object.hh"
#include "stack.hh"
#include "symbol_table.hh"
#include "nativeregistry.hh"

class VirtualMachine : public RootMarker {
private:
    File file;
    Heap heap;
    NativeFunctionRegistry registry;
    std::unique_ptr<StackFrame> frame;
public:
    VirtualMachine(File _file, std::uint64_t heap_size) 
    : file{std::move(_file)}
    , heap{heap_size}
    {
        registerNatives();
    }

    virtual ~VirtualMachine() = default;

    void Run() {
        DEBUGLN("Beginning vm");
        DEBUGLN("Adding root marker");
        heap.AddRootMarker(this);
        DEBUGLN("Adding initial frame");
        pushFrame(getEntrypoint());
        DEBUGLN("Entering main loop");
        loop();
        DEBUGLN("Main loop terminated");
    }

    void Mark(Heap* heap) override {
        DEBUGLN("Marking root objects");
        StackFrame* frame = this->frame.get();
        while (frame != nullptr) {
            DEBUGLN("Marking stack frame roots");
            frame->Mark(heap);
            frame = frame->GetNullableOuter();
        }
        DEBUGLN("Finished marking root objects");
    }

    void RegisterNativeFunction(NativeFunction fn) {
        this->registry.Register(std::move(fn));
    }

private:
    void loop() {
        if (IS_DEBUG_ENABLED()) {
            DEBUGLN("VM START");
            debugPrint();
            waitForInput();
        }

        while (true) {
            std::size_t pc = this->frame->GetProgramCounter();

            DEBUGLN("PC: " << pc);

            const Bytecode& bc = this->frame->GetFunction()->GetBytecode().at(pc);

            DEBUGLN("Loaded bytecode: " << bc.GetTypeToString());

            bool terminate = applyBytecode(bc);
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

    bool applyBytecode(const Bytecode& bc) {
        bool terminate = false;

        struct Visitor : public BytecodeVisitor {
            VirtualMachine& self;
            bool& terminate;

            Visitor(VirtualMachine& _self, bool& _terminate)
            : self{_self}, terminate{_terminate}
            {}

            void OnJumpIfFalse(const Bytecode& bc) { DEBUGLN("JumpIfFalse"); self.JumpIfFalse(bc); }
            void OnJump(const Bytecode& bc) { DEBUGLN("Jump"); self.Jump(bc); }
            void OnLoadNil(const Bytecode& bc) { DEBUGLN("LoadNil"); self.LoadNil(bc); }
            void OnReturn(const Bytecode& bc) { DEBUGLN("Return"); self.Return(bc); }
            void OnLoadLocal(const Bytecode& bc) { DEBUGLN("LoadLocal"); self.LoadLocal(bc); }
            void OnStoreLocal(const Bytecode& bc) { DEBUGLN("StoreLocal"); self.StoreLocal(bc); }
            void OnLoadInteger(const Bytecode& bc) { DEBUGLN("LoadInteger"); self.LoadInteger(bc); }
            void OnLoadString(const Bytecode& bc) { DEBUGLN("LoadString"); self.LoadString(bc); }
            void OnLoadTrue(const Bytecode& bc) { DEBUGLN("LoadTrue"); self.LoadTrue(bc); }
            void OnLoadFalse(const Bytecode& bc) { DEBUGLN("LoadFalse"); self.LoadFalse(bc); }
            void OnInvokeNative(const Bytecode& bc) { DEBUGLN("InvokeNative"); self.InvokeNative(bc); }
            void OnInvokeFunction(const Bytecode& bc) { DEBUGLN("InvokeFunction"); self.InvokeFunction(bc); }
            void OnLoadUnsigned(const Bytecode& bc) { DEBUGLN("LoadUnsigned"); self.LoadUnsigned(bc); }
            void OnPop(const Bytecode& bc) { DEBUGLN("Pop"); self.Pop(bc); }
            void OnHalt(const Bytecode& bc) { DEBUGLN("Halt"); terminate = true; }
        };

        return terminate;
    }

    void LoadLocal(const Bytecode& bc) {
        this->frame->Push(this->frame->GetLocal(bc.GetUnsignedArg()));
        this->frame->AdvanceProgramCounter();
    }

    void StoreLocal(const Bytecode& bc) {
        this->frame->SetLocal(bc.GetUnsignedArg(), this->frame->Pop());
        this->frame->AdvanceProgramCounter();
    }

    void Pop(const Bytecode& bc) {
        this->frame->Pop();
        this->frame->AdvanceProgramCounter();
    }

    void LoadString(const Bytecode& bc) {
        const std::string& str = this->file.GetConstants().at(bc.GetUnsignedArg()).GetStringConstant();
        Object* result = this->heap.AllocateString(str);
        Object tmp;
        tmp.SetReference(result);
        this->frame->Push(tmp);
        this->frame->AdvanceProgramCounter();
    }

    void InvokeNative(const Bytecode& bc) {
        std::uint64_t num_args = this->frame->Pop().GetUnsignedInteger();
        const std::string& native_fn_name = this->file.GetConstants().at(bc.GetUnsignedArg()).GetStringConstant();
        const NativeFunction& fn = this->registry.Get(native_fn_name);
        if (fn.GetArity() != num_args) {
            // TODO, don't crash the vm
            throw std::runtime_error{std::string{"Arity mismatch on native function call"}};
        }
        fn.Apply(this);
        this->frame->AdvanceProgramCounter();
    }

    void InvokeFunction(const Bytecode& bc) {
        const Function& fn = this->file.GetFunctions().at(bc.GetUnsignedArg());
        std::uint64_t num_args = this->frame->Pop().GetUnsignedInteger();
        if (fn.GetArity() != num_args) {
            // TODO, don't crash the vm
            throw std::runtime_error{std::string{"Arity mismatch on function call"}};
        }
        pushFrame(&fn);
        // assign the arguments from the outer stack frame
        StackFrame* outer = this->frame->GetNullableOuter();
        std::size_t local_index = num_args - 1;
        for (std::uint64_t i = 0; i < num_args; i++)  {
            this->frame->SetLocal(local_index, outer->Pop());
            local_index--;
        }
    }

    void LoadUnsigned(const Bytecode& bc) {
        Object tmp;
        tmp.SetUnsignedInteger(bc.GetUnsignedArg());
        this->frame->Push(tmp);
        this->frame->AdvanceProgramCounter();
    }

    void JumpIfFalse(const Bytecode& bc) {
        Object res = this->frame->Pop();
        if (res.IsType(ObjectType::Boolean) && !res.GetBoolean()) {
            this->frame->SetProgramCounter(bc.GetUnsignedArg());
        } else {
            this->frame->AdvanceProgramCounter();
        }
    }

    void Jump(const Bytecode& bc) {
        this->frame->SetProgramCounter(bc.GetUnsignedArg());
    }

    void LoadTrue(const Bytecode& bc) {
        Object temp;
        temp.SetBoolean(true);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadFalse(const Bytecode& bc) {
        Object temp;
        temp.SetBoolean(false);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadNil(const Bytecode& bc) {
        this->frame->Push(Object{});
        this->frame->AdvanceProgramCounter();
    }

    void LoadInteger(const Bytecode& bc) {
        Object temp;
        temp.SetInteger(this->file.GetConstants().at(bc.GetUnsignedArg()).GetIntegerConstant());
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void Return(const Bytecode& bc) {
        Object ret = this->frame->Pop();
        this->frame = this->frame->GetOuter();
        this->frame->Push(ret);
        this->frame->AdvanceProgramCounter();
    }

    void pushFrame(const Function* fn) {
        this->frame = std::make_unique<StackFrame>(std::move(this->frame), fn);
    }

    const Function* getEntrypoint() {
        // TODO: separate this into some other validation class
        if (this->file.GetFunctions().size() == 0) {
            throw std::runtime_error{std::string{"No functions defined in file"}};
        }
        auto result = &this->file.GetFunctions().at(0);
        if (result->GetArity() != 0) {
            throw std::runtime_error{std::string{"Expected entrypoint function to have arity of 0"}};
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
            const Bytecode& bc = frame->GetFunction()->GetBytecode().at(i);
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

#endif // VM_H__