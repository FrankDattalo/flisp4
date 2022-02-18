#ifndef STRING_HH__
#define STRING_HH__

#include "lib/std.hh"
#include "object.hh"
#include "integer.hh"
#include "character.hh"

class String : public Object {
public:
    String(const std::string& str): Object(Object::Type::String, AllocationSize(str))
    {
        *length() = Integer(str.size());
        char* dest = chars();
        for (std::size_t i = 0; i < str.size(); i++) {
            dest[i] = str.at(i);
        }
    }

    Integer Length() const { 
        return *this->length()->AsConstInteger(); 
    }

    Character GetChar(Integer index) const {
        if (index.Value() >= Length().Value()) {
            throw std::runtime_error{"String index out of bounds"};
        }
        char* c = chars();
        return Character(c[index.Value()]);
    }

    constexpr static std::size_t MinAllocationSize() {
        return sizeof(Object) + sizeof(Primitive);
    }

    bool HasNext(std::size_t i) const {
        return false;
    }

    Primitive* Next(std::size_t i) const {
        throw std::runtime_error{"String.Next should never be called"};
    }

    static std::size_t AllocationSize(const std::string& str) {
        DEBUGLN("String size is " << str.size());
        std::size_t string_bytes = str.size() * sizeof(char) + MinAllocationSize();
        DEBUGLN("Unaligned allocation size is " << string_bytes);
        if (string_bytes % sizeof(Object) != 0) {
            // round up to alignment
            DEBUGLN("Rounding up to next alignment before: " << string_bytes);
            string_bytes = (string_bytes / sizeof(Object) + 1) * sizeof(Object);
            DEBUGLN("Rounding up to next alignment after: " << string_bytes);
        }
        return string_bytes;
    }
private:
    Primitive* length() const {
        const Primitive* const_head = reinterpret_cast<const Primitive*>(this);
        Primitive* head = const_cast<Primitive*>(const_head);
        return &head[1];
    }

    char* chars() const {
        const char* data = reinterpret_cast<const char*>(this);
        char* result = const_cast<char*>(data);
        return &result[MinAllocationSize()];
    }
};

static_assert(sizeof(String) == sizeof(Object));

#endif // STRING_HH__