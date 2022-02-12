#ifndef SYMBOL_TABLE_HH__
#define SYMBOL_TABLE_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "primitive.hh"
#include "map.hh"

namespace runtime {

class SymbolTable : public Structure<Object::Type::SymbolTable, 2> {
private:
    FIELD(0, id_to_string_lookup);
    FIELD(1, string_to_id_lookup);

public:
    SymbolTable(Primitive _id_to_string, Primitive _string_to_id) : Structure() {
        id_to_string_lookup() = _id_to_string;
        string_to_id_lookup() = _string_to_id;
    }

    ~SymbolTable() = default;

    static Primitive Intern(SymbolTable* self, Heap* heap, String* str);

    static Primitive ToString(SymbolTable* self, Primitive symbol) {
        return Map::Lookup(self->const_id_to_string(), symbol);
    }

private:
    Map* id_to_string() { return id_to_string_lookup().GetReference()->AsMap(); }
    const Map* const_id_to_string() const { return const_id_to_string_lookup().GetReference()->AsMap(); }

    Map* string_to_id() { return string_to_id_lookup().GetReference()->AsMap(); }
    const Map* const_string_to_id() { return const_string_to_id_lookup().GetReference()->AsMap(); }
};

static_assert(sizeof(SymbolTable) == sizeof(Object));


} // namespace runtime

#endif // SYMBOL_TABLE_HH__