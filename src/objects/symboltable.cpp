#include "objects/symboltable.hh"
#include "heap.hh"

namespace runtime {

Primitive SymbolTable::Intern(SymbolTable* self_, Heap* heap, Primitive str_) {

    Handle<SymbolTable> self = heap->GetHandle(self_);
    Handle<Primitive> str = heap->GetHandle(str_);

    Primitive lookup = Map::Lookup(self->string_to_id(), str);

    if (lookup.GetType() != Primitive::Type::Nil) {
        return lookup;
    }

    // use the current map size as the canonical symbol id
    std::int64_t map_size = Map::Size(self->string_to_id()).GetInteger();
    std::uint64_t symbol_id = map_size;
    Primitive symbol = Primitive::Symbol(symbol_id);

    Map::Insert(
        self->string_to_id(),
        heap, str.GetData(), symbol
    );

    Map::Insert(
        self->id_to_string(),
        heap, symbol, str.GetData()
    );

    return symbol;
}

} // namespace runtime