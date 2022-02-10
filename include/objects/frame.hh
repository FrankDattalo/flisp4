#ifndef FRAME_HH__
#define FRAME_HH__

#include "structure.hh"

namespace runtime {

class Frame : public Structure<Object::Type::Frame, 3> {
private:
    FIELD(0, env);
    FIELD(1, outer);
    FIELD(2, stack);
public:
    Frame(Primitive _env, Primitive _outer, Primitive _stack) : Structure() {
        env() = _env;
        outer() = _outer;
        stack() = _stack;
    }
};

} // namespace runtime

#endif // FRAME_HH__