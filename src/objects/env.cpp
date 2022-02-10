#include "objects/env.hh"

namespace runtime {

void Envrionment::Define(Heap* heap, Primitive symbol, Primitive value) {
    if (symbol.GetType() != Primitive::Type::Symbol) {
        throw std::runtime_error{"Variable definition must be a symbol"};
    }
    lookup().GetReference()->AsMap()->Insert(heap, symbol, value);
}

} // namespace runtime