#ifndef SYMBOL_HH__
#define SYMBOL_HH__

#include "lib.hh"
#include "util.hh"
#include "primitive.hh"

class Symbol : public Primitive {
public:
    Symbol(std::uint64_t value) {
        SetSymbol(value);
    }

    ~Symbol() = default;

    std::uint64_t Value() const {
        return GetSymbol();
    }
};

#endif // SYMBOL_HH__