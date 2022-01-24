#ifndef MEMORY_SEMANTIC_MACROS_HH__
#define MEMORY_SEMANTIC_MACROS_HH__

#define MOVEABLE(ClassName) \
    ClassName(ClassName&&) = default; \
    ClassName&& operator=(ClassName&&) = default;

#define NOT_MOVEABLE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName&& operator=(ClassName&&) = delete;

#define COPYABLE(ClassName) \
    ClassName(const ClassName&) = default; \
    ClassName& operator=(const ClassName&) = default;

#define NOT_COPYABLE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;

#endif // MEMORY_SEMANTIC_MACROS_HH__