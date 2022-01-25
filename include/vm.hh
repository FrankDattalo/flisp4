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
                    LoadLocal(pc, bc);
                    break;
                }
                case BytecodeType::StoreLocal: {
                    DEBUGLN("StoreLocal");
                    StoreLocal(pc, bc);
                    break;
                }
                case BytecodeType::JumpIfFalse: {
                    DEBUGLN("JumpIfFalse");
                    JumpIfFalse(pc, bc);
                    break;
                }
                case BytecodeType::Jump: {
                    DEBUGLN("Jump");
                    Jump(pc, bc);
                    break;
                }
                case BytecodeType::Invoke: {
                    DEBUGLN("Invoke");
                    Invoke(pc, bc);
                    break;
                }
                case BytecodeType::LoadTrue: {
                    DEBUGLN("LoadTrue");
                    LoadTrue(pc, bc);
                    break;
                }
                case BytecodeType::LoadFalse: {
                    DEBUGLN("LoadFalse");
                    LoadFalse(pc, bc);
                    break;
                }
                case BytecodeType::LoadNil: {
                    DEBUGLN("LoadNil");
                    LoadNil(pc, bc);
                    break;
                }
                case BytecodeType::LoadInteger: {
                    DEBUGLN("LoadInteger");
                    LoadInteger(pc, bc);
                    break;
                }
                case BytecodeType::LoadSymbol: {
                    DEBUGLN("LoadSymbol");
                    LoadSymbol(pc, bc);
                    break;
                }
                case BytecodeType::LoadCharacter: {
                    DEBUGLN("LoadCharacter");
                    LoadCharacter(pc, bc);
                    break;
                }
                case BytecodeType::LoadField: {
                    DEBUGLN("LoadField");
                    LoadField(pc, bc);
                    break;
                }
                case BytecodeType::StoreField: {
                    DEBUGLN("StoreField");
                    StoreField(pc, bc);
                    break;
                }
                case BytecodeType::Return: {
                    DEBUGLN("Return");
                    Return(pc, bc);
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
                DEBUGLN("VM AFTER");
                debugPrint();
            }
        }
    }

    void LoadLocal(std::uint64_t pc, const Bytecode& bc) {
        this->frame->Push(this->frame->GetLocal(bc.GetArg()));
        this->frame->AdvanceProgramCounter();
    }

    void StoreLocal(std::uint64_t pc, const Bytecode& bc) {
        this->frame->SetLocal(bc.GetArg(), this->frame->Pop());
        this->frame->AdvanceProgramCounter();
    }

    void JumpIfFalse(std::uint64_t pc, const Bytecode& bc) {
        Object res = this->frame->Pop();
        if (res.IsType(ObjectType::Boolean) && !res.GetBoolean()) {
            this->frame->SetProgramCounter(bc.GetArg());
        } else {
            this->frame->AdvanceProgramCounter();
        }
    }

    void Jump(std::uint64_t pc, const Bytecode& bc) {
        this->frame->SetProgramCounter(bc.GetArg());
    }

    void Invoke(std::uint64_t pc, const Bytecode& bc) {
        // TODO
    }

    void LoadTrue(std::uint64_t pc, const Bytecode& bc) {
        Object temp;
        temp.SetBoolean(true);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadFalse(std::uint64_t pc, const Bytecode& bc) {
        Object temp;
        temp.SetBoolean(false);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadNil(std::uint64_t pc, const Bytecode& bc) {
        this->frame->Push(Object{});
        this->frame->AdvanceProgramCounter();
    }

    void LoadInteger(std::uint64_t pc, const Bytecode& bc) {
        Object temp;
        temp.SetInteger(this->file.GetIntegerConstants().at(bc.GetArg()));
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadSymbol(std::uint64_t pc, const Bytecode& bc) {
        // TODO
        Object temp;
        const std::string& str = this->file.GetSymbolConstants().at(bc.GetArg());
        std::uint64_t symbol_id = this->symbol_table.Intern(str);
        temp.SetSymbol(symbol_id);
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadCharacter(std::uint64_t pc, const Bytecode& bc) {
        Object temp;
        temp.SetCharacter(this->file.GetCharacterConstants().at(bc.GetArg()));
        this->frame->Push(temp);
        this->frame->AdvanceProgramCounter();
    }

    void LoadField(std::uint64_t pc, const Bytecode& bc) {
        // TODO
    }

    void StoreField(std::uint64_t pc, const Bytecode& bc) {
        // TODO
    }

    void Return(std::uint64_t pc, const Bytecode& bc) {
        // TODO
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
            if (bc.HasArg()) {
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
    }
};

#endif // VM_H__