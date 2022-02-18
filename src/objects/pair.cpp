#include "objects/stack.hh"
#include "heap.hh"

Pair::Pair(Handle _first, Handle _second) : Structure() {
    First() = _first;
    Second() = _second;
}