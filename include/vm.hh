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

class VirtualMachine : public RootMarker {
private:
    File file;
    Heap heap;
    SymbolTable symbol_table;
    std::unique_ptr<StackFrame> frame;
public:
    VirtualMachine(File _file, std::uint64_t heap_size) 
    : file{std::move(_file)}
    , heap{heap_size}
    {}

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

private:
    void loop() {
        while (true) {
            if (IS_DEBUG_ENABLED()) {
                DEBUGLN("VM BEFORE");
                debugPrint();
            }

            std::size_t pc = this->frame->GetProgramCounter();

            DEBUGLN("PC: " << pc);

            const Bytecode& bc = this->frame->GetFunction()->GetBytecode().at(pc);

            DEBUGLN("Loaded bytecode: " << static_cast<std::uint64_t>(bc.GetType()));

            switch (bc.GetType()) {
                case BytecodeType::LoadLocal: {
                    DEBUGLN("LoadLocal");
                    LoadLocal(bc);
                    break;
                }
                case BytecodeType::StoreLocal: {
                    DEBUGLN("StoreLocal");
                    StoreLocal(bc);
                    break;
                }
                case BytecodeType::JumpIfFalse: {
                    DEBUGLN("JumpIfFalse");
                    JumpIfFalse(bc);
                    break;
                }
                case BytecodeType::Jump: {
                    DEBUGLN("Jump");
                    Jump(bc);
                    break;
                }
                case BytecodeType::Invoke: {
                    DEBUGLN("Invoke");
                    Invoke(bc);
                    break;
                }
                case BytecodeType::LoadTrue: {
                    DEBUGLN("LoadTrue");
                    LoadTrue(bc);
                    break;
                }
                case BytecodeType::LoadFalse: {
                    DEBUGLN("LoadFalse");
                    LoadFalse(bc);
                    break;
                }
                case BytecodeType::LoadNil: {
                    DEBUGLN("LoadNil");
                    LoadNil(bc);
                    break;
                }
                case BytecodeType::LoadInteger: {
                    DEBUGLN("LoadInteger");
                    LoadInteger(bc);
                    break;
                }
                case BytecodeType::LoadSymbol: {
                    DEBUGLN("LoadSymbol");
                    LoadSymbol(bc);
                    break;
                }
                case BytecodeType::LoadCharacter: {
                    DEBUGLN("LoadCharacter");
                    LoadCharacter(bc);
                    break;
                }
                case BytecodeType::LoadField: {
                    DEBUGLN("LoadField");
                    LoadField(bc);
                    break;
                }
                case BytecodeType::StoreField: {
                    DEBUGLN("StoreField");
                    StoreField(bc);
                    break;
                }
                case BytecodeType::Return: {
                    DEBUGLN("Return");
                    Return(bc);
                    break;
                }
                case BytecodeType::MakeFunction: {
                    DEBUGLN("MakeFunction");
                    MakeFunction(bc);
                    break;
                }
                case BytecodeType::Halt: {
                    DEBUGLN("Halt");
                    return;
                }
                default: {
                    std::string error_message{"Unknown bytecode in VirtualMachine::loop "};
                    error_message.append(std::to_string(static_cast<std::uint64_t>(bc.GetType())));
                    throw std::runtime_error{error_message};
                }
            }

            if (IS_DEBUG_ENABLED()) {
                waitForInput();
            }
        }
    }

    void LoadLocal(const Bytecode& bc) {
        this->frame->Push(this->frame->GetLocal(bc.GetArg()));
        this->frame->AdvanceProgramCounter();
    }

    void StoreLocal(const Bytecode& bc) {
        this->frame->SetLocal(bc.GetArg(), this->frame->Pop());
        this->frame->AdvanceProgramCounter();
    }

    void JumpIfFalse(const Bytecode& bc) {
        Object res = this->frame->Pop();
        if (res.IsType(ObjectType::Boolean) && !res.GetBoolean()) {
            this->frame->SetProgramCounter(bc.GetArg());
        } else {
            this->frame->AdvanceProgramCounter();
        }
    }

    void Jump(const Bytecode& bc) {
        this->frame->SetProgramCounter(bc.GetArg());
    }

    void Invoke(const Bytecode& bc) {
        std::uint64_t num_args = bc.GetArg();
        Object reciever = this->frame->OffsetFromTop(num_args);
        if (!reciever.IsType(ObjectType::FunctionReference)) {
            // TODO, don't crash the vm
            throw std::runtime_error{std::string{"Expected reciever to be a function"}};
        }
        const Function* fn = reciever.GetFunctionReference();
        if (fn->GetArity() != num_args) {
            // TODO, don't crash the vm
            throw std::runtime_error{std::string{"Arity mismatch"}};
        }
        pushFrame(fn);
        StackFrame* outer = this->frame->GetNullableOuter();
        std::size_t local_index = num_args - 1;
        for (std::uint64_t i = 0; i < num_args; i++)  {
            this->frame->SetLocal(local_index, outer->Pop());
            local_index--;
        }
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
        temp.SetInteger(this->file.GetIntegerConstants().at(bc.GetArg()));
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadSymbol(const Bytecode& bc) {
        // TODO
        Object temp;
        const std::string& str = this->file.GetSymbolConstants().at(bc.GetArg());
        std::uint64_t symbol_id = this->symbol_table.Intern(str);
        temp.SetSymbol(symbol_id);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadCharacter(const Bytecode& bc) {
        Object temp;
        temp.SetCharacter(this->file.GetCharacterConstants().at(bc.GetArg()));
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadField(const Bytecode& bc) {
        // TODO
    }

    void StoreField(const Bytecode& bc) {
        // TODO
    }

    void Return(const Bytecode& bc) {
        Object ret = this->frame->Pop();
        this->frame = this->frame->GetOuter();
        this->frame->Push(ret);
        this->frame->AdvanceProgramCounter();
    }

    void MakeFunction(const Bytecode& bc) {
        const Function* fn = &this->file.GetFunctions().at(bc.GetArg());
        Object tmp;
        tmp.SetFunctionReference(fn);
        this->frame->Push(tmp);
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
            std::cout << Bytecode::TypeToString(bc.GetType());
            if (Bytecode::HasArg(bc.GetType())) {
                std::cout << "(" << bc.GetArg() << ")";
            }
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
};

#endif // VM_H__