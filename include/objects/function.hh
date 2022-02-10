#ifndef FUNCTION_HH__
#define FUNCTION_HH__

#include "structure.hh"

namespace runtime {

class Function : public Structure<Object::Type::Function, 2> {
private:
    FIELD(0, code);
    FIELD(1, formals);
public:
    Function(Primitive _code, Primitive _formals) : Structure() {
        code() = _code;
        formals() = _formals;
    }
};

};

#endif // FUNCTION_HH__ 