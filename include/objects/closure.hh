#ifndef CLOSURE_HH__
#define CLOSURE_HH__

#include "structure.hh"

namespace runtime {

class Closure : public Structure<Object::Type::Closure, 2> {
private:
    FIELD(0, function);
    FIELD(1, env);
public:
    Closure(Primitive _function, Primitive _env) : Structure() {
        function() = _function;
        env() = _env;
    }
};

};

#endif // FUNCTION_HH__ 