#ifndef ENV_HH__
#define ENV_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "map.hh"

class Envrionment : public Structure<Object::Type::Envrionment, 2> {
public:
    Envrionment(Handle _outer, Handle _lookup);

    ~Envrionment() = default;

    FIELD(0, Outer);

    FIELD(1, Lookup);
};

static_assert(sizeof(Envrionment) == sizeof(Object));

#endif // ENV_HH__