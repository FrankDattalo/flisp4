#include "objects/env.hh"

namespace runtime {

void Envrionment::Define(Envrionment* self, Heap* heap, Primitive symbol, Primitive value) {
    if (symbol.GetType() != Primitive::Type::Symbol) {
        throw std::runtime_error{"Variable definition must be a symbol"};
    }
    Map::Insert(self->lookup(), heap, symbol, value);
}

} // namespace runtime