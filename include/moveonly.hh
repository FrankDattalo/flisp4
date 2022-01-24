#ifndef MOVEONLY_HH__
#define MOVEONLY_HH__

#include "memory_semantic_macros.hh"

// this class is not currently used, candidate to delete

template <typename T>
class MoveOnly {
private:
    T data;
public:
    MoveOnly(T _data)
    : data{std::move(_data)}
    {}

    ~MoveOnly() = default;

    NOT_COPYABLE(MoveOnly);

    MOVEABLE(MoveOnly);

    T& Data() {
        return this->data;
    }
};

#endif // MOVEONLY_HH__