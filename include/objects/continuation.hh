#ifndef CONTINUATION_HH__
#define CONTINUATION_HH__

#include "structure.hh"

class Continuation : public Structure<Object::Type::Continuation, 1> {
public:
    Continuation(Handle _frame);

    ~Continuation() = default;

    FIELD(0, Frame);
};

#endif // CONTINUATION_HH__