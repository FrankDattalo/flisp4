#ifndef ENV_HH__
#define ENV_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "map.hh"

namespace runtime {

class Envrionment : public Structure<Object::Type::Envrionment, 2> {
private:
    FIELD(0, outer);
    FIELD(1, lookup_slot);

public:
    Envrionment(Primitive _outer, Primitive _lookup) : Structure() {
        outer() = _outer;
        lookup_slot() = _lookup;
    }

    ~Envrionment() = default;

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

    static void Define(Envrionment* self, Heap* heap, Primitive symbol, Primitive value);
private:
    Map* lookup() {
        return lookup_slot().GetReference()->AsMap();
    }
};

static_assert(sizeof(Envrionment) == sizeof(Object));

} // namespace runtime

#endif // ENV_HH__