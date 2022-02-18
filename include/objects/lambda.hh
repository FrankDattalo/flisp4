#ifndef LAMBDA_HH__
#define LAMBDA_HH__

#include "structure.hh"

class Lambda : public Structure<Object::Type::Lambda, 3> {
public:
    Lambda(Handle _args, Handle _env, Handle _bytecode);

    FIELD(0, Function);

    FIELD(1, Env);

    FIELD(2, Bytecode);
};

#endif // LAMBDA_HH__