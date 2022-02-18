#ifndef MAP_HH__
#define MAP_HH__

#include "lib/std.hh"
#include "structure.hh"
#include "pair.hh"

class Map : public Structure<Object::Type::Map, 1> {
public:
    Map();

    ~Map() = default;

    FIELD(0, Head);
};

static_assert(sizeof(Map) == sizeof(Object));

#endif // MAP_HH__