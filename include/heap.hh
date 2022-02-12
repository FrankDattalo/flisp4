#ifndef HEAP_HH__
#define HEAP_HH__

#include "lib.hh"
#include "util.hh"
#include "objects.hh"

namespace runtime {

class SemiSpaceIterator;

class SemiSpace {
friend SemiSpaceIterator;
private:
    char* data;
    std::uint64_t data_size;
    std::uint64_t first_free;
public:
    SemiSpace(std::uint64_t size) {
        data = new char[size];
        data_size = size;
        first_free = 0;
    }

    ~SemiSpace() {
        delete[] data;
    }

    NOT_COPYABLE(SemiSpace);

    NOT_MOVEABLE(SemiSpace);

    bool CanFit(std::size_t bytes) {
        bool result = AvailableSlots() >= bytes;
        DEBUGLN("CanFit(" << bytes << ")? = " << result);
        return result;
    }

    void* Allocate(std::size_t bytes) {
        DEBUGLN("Semispace allocation " << bytes);
        if (!CanFit(bytes)) {
            throw std::runtime_error{std::string{"Tried to allocate without enough space"}};
        }
        void* addr = &data[first_free];
        first_free += bytes;
        DEBUGLN("Semispace has " << AvailableSlots() << " bytes free");
        return addr;
    }

    void Clear() {
        DEBUGLN("Clearning semispace");
        first_free = 0;
    }

    bool Owns(void* ptr) {
        return &this->data[0] <= ptr && ptr <= &this->data[data_size-1];
    }

    SemiSpaceIterator Iterator();

private:
    std::size_t AvailableSlots() {
        return this->data_size - this->first_free;
    }

    std::size_t FirstFree() {
        return this->first_free;
    }

    void* At(std::size_t index) {
        return &this->data[index];
    }
};

class SemiSpaceIterator {
private:
    SemiSpace* space;
    std::size_t next_index = 0;
public:
    SemiSpaceIterator(SemiSpace* _space)
    : space{_space}
    {}

    NOT_COPYABLE(SemiSpaceIterator);
    NOT_MOVEABLE(SemiSpaceIterator);

    ~SemiSpaceIterator() = default;

    bool HasNext() const {
        bool result = next_index < space->FirstFree();
        DEBUGLN("SemiSpace.HasNext? " << next_index << " < " << space->FirstFree() << " = " << result);
        return result;
    }

    Object* Next() {
        void* addr = space->At(next_index);
        DEBUGLN("Semispace next = " << addr);
        Object* casted = reinterpret_cast<Object*>(addr);
        std::size_t alloc_size = casted->GetAllocationSize();
        DEBUGLN("Allocation size for " << addr << " was " << alloc_size);
        next_index += alloc_size;
        DEBUGLN("New next index: " << next_index);
        return casted;
    }
};

class Heap;
class RootManager;

class UntypedHandle {
protected:
    Primitive location;
    RootManager* manager;
public:
    UntypedHandle(RootManager* _manager, Primitive _location);

    virtual ~UntypedHandle();

    NOT_MOVEABLE(UntypedHandle);

    // TODO custom copy constructor and operator
    NOT_COPYABLE(UntypedHandle);

    const Primitive & GetData() const { return location; }
};

class PrimitiveHandle : public UntypedHandle {
public:
    PrimitiveHandle(RootManager* _manager, Primitive _data)
    : UntypedHandle(_manager, _data)
    {}

    virtual ~PrimitiveHandle() = default;

    Primitive* operator->() {
        return &location;
    }
};

template<typename T>
class ReferenceHandle : public UntypedHandle {
public:
    ReferenceHandle(RootManager* _manager, T* ref)
    : UntypedHandle(_manager, Primitive::Reference(ref))
    {}

    virtual ~ReferenceHandle() = default;

    T* operator->() {
        return GetPointer();
    }

    T* GetPointer() {
        return location.GetReference()->As<T>();
    }
};

class RootManager {
private:
    std::set<Primitive*> roots;
public:
    RootManager() = default;
    ~RootManager() = default;

    NOT_COPYABLE(RootManager);
    NOT_MOVEABLE(RootManager);

    template<typename T>
    ReferenceHandle<T> Get(T* ref) {
        ReferenceHandle<T> ret{this, ref};
        return ret;
    }

    PrimitiveHandle Get(Primitive val) {
        PrimitiveHandle ret{this, val};
        return ret;
    }

    void AddRoot(Primitive* root) {
        DEBUGLN("Adding root at " << root);
        roots.insert(root);
    }

    void RemoveRoot(Primitive* root) {
        DEBUGLN("Removing root at " << root);
        roots.erase(root);
    }

    const std::set<Primitive*>& GetRoots() const {
        return roots;
    }
};

class Heap {
private:
    static constexpr std::size_t ALIGNMENT = 8;
    SemiSpace space1;
    SemiSpace space2;
    SemiSpace* active;
    SemiSpace* passive;
    RootManager roots;
public:
    Heap(std::size_t size) : space1{size}, space2{size} {
        active = &space1;
        passive = &space2;
    }

    ~Heap() = default;

    NOT_COPYABLE(Heap);

    NOT_MOVEABLE(Heap);

    template<typename T>
    ReferenceHandle<T> GetHandle(T* ptr) {
        return roots.Get(ptr);
    }

    PrimitiveHandle GetHandle(Primitive val) {
        return roots.Get(val);
    }

    template<typename T, typename... Primitives>
    T* StructureAllocator(const Primitives&... args) {
        void* addr = Allocate(T::AllocationSize());
        T* ret = new (addr) T(args...);
        return ret;
    }

    Vector* NewVector(std::size_t items) {
        void* addr = Allocate(Vector::AllocationSize(items));
        Vector* ret = new (addr) Vector(items);
        return ret;
    }

    String* NewString(const std::string& str) {
        void* addr = Allocate(String::AllocationSize(str));
        String* ret = new (addr) String(str);
        return ret;
    }

    Pair* NewPair(const Primitive & first, const Primitive & second) {
        return StructureAllocator<Pair>(first, second);
    }

    Map* NewMap() {
        return StructureAllocator<Map>();
    }

    Envrionment* NewEnvironment(const Primitive & outer, const Primitive & lookup) {
        return StructureAllocator<Envrionment>(outer, lookup);
    }

    Stack* NewStack() {
        return StructureAllocator<Stack>();
    }

    Frame* NewFrame(const Primitive & env, const Primitive & outer, const Primitive & stack) {
        return StructureAllocator<Frame>(env, outer, stack);
    }

    VirtualMachine* NewVirtualMachine() {
        return StructureAllocator<VirtualMachine>(Primitive::NativeReference(this));
    }

    NativeFunction* NewNativeFunction(NativeFunctionPointer ptr, std::int64_t arity) {
        return StructureAllocator<NativeFunction>(
            Primitive::NativeReference(reinterpret_cast<void*>(ptr)),
            Primitive::Integer(arity)
        );
    }

    SymbolTable* NewSymbolTable(const Primitive & map1, const Primitive & map2) {
        return StructureAllocator<SymbolTable>(map1, map2);
    }

private:
    void* Allocate(std::size_t bytes) {

        DEBUGLN("Allocating " << bytes);

        if (bytes % ALIGNMENT != 0) {
            throw std::runtime_error{"Cannot allocated unaligned bytes"};
        }

        if (active->CanFit(bytes)) {
            DEBUGLN("Active semispace can fit");
            return active->Allocate(bytes);
        }

        DEBUGLN("Gc needed");

        Gc();

        DEBUGLN("Gc done, trying allocating again");

        if (active->CanFit(bytes)) {
            DEBUGLN("Active semispace can fit after gc");
            return active->Allocate(bytes);
        }

        DEBUGLN("OOM");

        throw std::runtime_error{std::string{"Out of memory"}};
    }

    void mark() {
        DEBUGLN("Marking roots");
        auto iter = roots.GetRoots();
        for (Primitive* root : iter) {
            DEBUGLN("Visiting root at " << root);
            transferIfReference(root);
        }
    }

    void transferIfReference(Primitive* location) {
        struct Visitor : public Primitive::Visitor {
            Heap* heap;
            Primitive* location;

            Visitor(Heap* _heap, Primitive* _location)
            : heap{_heap}, location{_location} {}

            virtual ~Visitor() = default;

            void OnNil(const Primitive*) override {/* intentionally empty */}
            void OnInteger(const Primitive*) override {/* intentionally empty */}
            void OnSymbol(const Primitive*) override {/* intentionally empty */}
            void OnReal(const Primitive*) override {/* intentionally empty */}
            void OnBoolean(const Primitive*) override {/* intentionally empty */}
            void OnNativeReference(const Primitive*) override {/* intentionally empty */}
            void OnCharacter(const Primitive*) override {/* intentionally empty */}
            void OnReference(const Primitive*) override {
                DEBUGLN("Reference object detected, moving to new space");
                heap->transferReference(location);
            }
        } visitor(this, location);

        location->Visit(visitor);
    }

    void transferReference(Primitive* location) {
        Object* ref = location->GetReference();

        // if it's already been moved, just update the location with the
        // new pointer
        if (ref->IsGcForward()) {
            DEBUGLN("Gc forward detected, " << ref << " was already moved moved to " << ref->GetGcForwardAddress());
            location->SetReference(ref->GetGcForwardAddress());
            return;
        }

        // otherwise, move the object and then update the location
        // with the new pointer
        std::size_t allocation_size = ref->GetAllocationSize();
        DEBUGLN("Moving object with allocation size " << allocation_size);
        void* new_addr = active->Allocate(allocation_size);
        DEBUGLN("New address for " << ref << " is " << new_addr);
        Object* new_addr_casted = reinterpret_cast<Object*>(new_addr);
        memcpy(new_addr_casted, ref, allocation_size);
        DEBUGLN("Copied over contents");
        ref->SetGcForwardAddress(new_addr_casted);
        DEBUGLN("Old address " << ref << " now forwarding to " << new_addr_casted);
        location->SetReference(new_addr_casted);
        DEBUGLN(location << " updated to point to " << new_addr_casted);
    }

    void transfer();

    // Actual GC Implementation here
    void Gc() {
        // swap the spaces 
        DEBUGLN("Swapping semispaces");
        SemiSpace* temp = active;
        active = passive;
        passive = temp;

        // marks all the roots, transferring them to the
        // opposite space in the process
        mark();

        // iterate over all the newly transferred things
        // and pull over all their children
        transfer();

        // gc the passive size
        DEBUGLN("Clearing old heap");
        passive->Clear();
    }

};

} // namespace runtime

#endif // HEAP_H__