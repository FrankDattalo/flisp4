#include "refactor/heap.hh"

namespace runtime {

SemiSpaceIterator SemiSpace::Iterator() {
    return SemiSpaceIterator{this};
}

Handle HandleManager::Get() {
    Handle ret{this};
    return ret;
}

} // namespace runtime