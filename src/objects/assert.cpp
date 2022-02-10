#include "objects.hh"

namespace runtime {

// sanity that we can always write a gc forward for any object
#define ASSERT_SIZE(v) static_assert(v::MinAllocationSize() >= Object::RequiredMinAllocationSize());
PER_CONCRETE_OBJECT_TYPE(ASSERT_SIZE)
#undef ASSERT_SIZE

} // namespace runtime