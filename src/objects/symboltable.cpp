#include "objects/symboltable.hh"
#include "heap.hh"

namespace runtime {

Primitive SymbolTable::Intern(Heap* heap, Primitive str) {
    Primitive lookup = string_to_id()->Lookup(str);

    if (lookup.GetType() != Primitive::Type::Nil) {
        return lookup;
    }

    Handle this_handle = heap->GetHandle(Primitive::Reference(this));
    Handle str_handle = heap->GetHandle(str);

    // use the current map size as the canonical symbol id
    std::int64_t map_size = string_to_id()->Size().GetInteger();
    std::uint64_t symbol_id = map_size;
    Primitive symbol = Primitive::Symbol(symbol_id);

    string_to_id()->Insert(heap, str, symbol);
    id_to_string()->Insert(heap, symbol, str);

    return symbol;
}

} // namespace runtime