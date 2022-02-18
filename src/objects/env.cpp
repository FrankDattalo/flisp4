#include "objects/env.hh"

/*
    static Primitive Lookup(Envrionment* self, Primitive symbol) {
        if (symbol.GetType() != Primitive::Type::Symbol) {
            throw std::runtime_error{"Variable lookup must be a symbol"};
        }
        Primitive env = Primitive::Reference(self);
        while (env.GetType() != Primitive::Type::Nil) {
            Envrionment* e = env.GetReference()->AsEnvrionment();
            Primitive result = Map::Lookup(e->lookup(), symbol);
            if (result.GetType() != Primitive::Type::Nil) {
                return result;
            }
            env = e->outer();
        }
        return Primitive::Nil();
    }

void Envrionment::Define(Envrionment* self, Heap* heap, Primitive symbol, Primitive value) {
    if (symbol.GetType() != Primitive::Type::Symbol) {
        throw std::runtime_error{"Variable definition must be a symbol"};
    }
    Map::Insert(self->lookup(), heap, symbol, value);
}
*/