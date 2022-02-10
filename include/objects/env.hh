#ifndef ENV_HH__
#define ENV_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "map.hh"

namespace runtime {

class Envrionment : public Structure<Object::Type::Envrionment, 2> {
private:
    FIELD(0, outer);
    FIELD(1, lookup);

public:
    Envrionment(Primitive _outer, Primitive _lookup) : Structure() {
        outer() = _outer;
        lookup() = _lookup;
    }

    ~Envrionment() = default;

    Primitive Lookup(Primitive symbol) const {
        if (symbol.GetType() != Primitive::Type::Symbol) {
            throw std::runtime_error{"Variable lookup must be a symbol"};
        }
        Primitive env = Primitive::Reference(const_cast<Envrionment*>(this));
        while (env.GetType() != Primitive::Type::Nil) {
            Envrionment* e = env.GetReference()->AsEnvrionment();
            Primitive result = e->lookup().GetReference()->AsMap()->Lookup(symbol);
            if (result.GetType() != Primitive::Type::Nil) {
                return result;
            }
            env = e->outer();
        }
        return Primitive::Nil();
    }

    void Define(Heap* heap, Primitive symbol, Primitive value);
};

static_assert(sizeof(Envrionment) == sizeof(Object));

} // namespace runtime

#endif // ENV_HH__