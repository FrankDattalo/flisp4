#ifndef FRAME_HH__
#define FRAME_HH__

#include "structure.hh"
#include "integer.hh"
#include "vector.hh"

class Frame : public Structure<Object::Type::Frame, 5> {
public:
    Frame(Handle _bytecode, Handle _outer, Handle _temps, Handle _env);

    ~Frame() = default;

    FIELD(0, Bytecode);

    FIELD(1, Outer);

    FIELD(2, Temps);

    FIELD(3, Env);

    FIELD(4, ProgramCounter);

    Integer BytecodeLength() const {
        return ConstBytecodeVector()->Length();
    }

    Primitive NextBytecode() const {
        Integer pc = *ConstProgramCounter().AsConstInteger();
        const Vector* v = ConstBytecodeVector();
        return v->GetItem(pc);
    }

private:
    const Vector* ConstBytecodeVector() const {
        return ConstBytecode().AsConstReference()->Value()->AsConstVector();
    }
};

static_assert(sizeof(Frame) == sizeof(Object));

#endif // FRAME_HH__