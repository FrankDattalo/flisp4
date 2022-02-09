#ifndef ENV_HH__
#define ENV_HH__

#include "lib/std.hh"
#include "structure.hh"

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

    Map* GetLookup() const;
    Primitive GetOuter() const;
};

static_assert(sizeof(Envrionment) == sizeof(Object));

} // namespace runtime

#endif // ENV_HH__