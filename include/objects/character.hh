#ifndef CHARACTER_HH__
#define CHARACTER_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class Character : public Primitive {
public:
    Character(char value) {
        SetCharacter(value);
    }

    ~Character() = default;

    char Value() const {
        return GetCharacter();
    }
};

#endif // CHARACTER_HH__