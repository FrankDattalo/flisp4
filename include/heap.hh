#ifndef HEAP_HH__
#define HEAP_HH__

#include "lib.hh"
#include "util.hh"
#include "objects.hh"

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

class RootManager {
private:
    std::set<Primitive*> roots;
public:
    RootManager() = default;
    ~RootManager() = default;

    NOT_COPYABLE(RootManager);
    NOT_MOVEABLE(RootManager);

    void AddRoot(Primitive* data) {
        roots.insert(data);
    }

    void RemoveRoot(Primitive* data) {
        roots.erase(data);
    }

    const std::set<Primitive*>& GetRoots() const {
        return roots;
    }
};

class HandleBlock {
    RootManager* manager;
    Primitive data;
public:
    HandleBlock(RootManager* _manager, Primitive _data) {
        this->manager = _manager;
        this->data = _data;
        this->manager->AddRoot(&this->data);
    }

    ~HandleBlock() {
        this->manager->RemoveRoot(&this->data);
    }

    NOT_COPYABLE(HandleBlock);
    NOT_MOVEABLE(HandleBlock);

    Primitive Data() const {
        return data;
    }

    Primitive* DataPtr() {
        return &data;
    }
};

class Handle {
    std::shared_ptr<HandleBlock> block;

public:
    Handle(std::shared_ptr<HandleBlock> _block)
    : block{_block}
    {}

    Handle() 
    : block{nullptr}
    {}

    ~Handle() = default;

    COPYABLE(Handle);

    MOVEABLE(Handle);

    void AssignTo(Primitive& location) const {
        location = block->Data();
    }

    #define DEFINE_CASTERS(V) \
        V* As##V() { \
            return block->DataPtr()->AsReference()->Value()->As##V(); \
        }
    PER_CONCRETE_OBJECT_TYPE(DEFINE_CASTERS)
    #undef DEFINE_CASTERS

    #define DEFINE_PRIMITIVE_CASTERS(V) \
        V* As##V() { \
            return block->DataPtr()->As##V(); \
        }
    PER_PRIMITIVE_TYPE(DEFINE_PRIMITIVE_CASTERS)
    #undef DEFINE_PRIMITIVE_CASTERS
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

    Handle GetHandle(Primitive val) {
        std::shared_ptr<HandleBlock> hb = std::make_shared<HandleBlock>(&roots, val);
        Handle ret{hb};
        return ret;
    }

    template<typename T, typename... Handles>
    Handle StructureAllocator(Handles... args) {
        void* addr = Allocate(T::AllocationSize());
        T* ptr = new (addr) T(args...);
        std::shared_ptr<HandleBlock> hb = std::make_shared<HandleBlock>(&roots, Reference(ptr));
        Handle ret{hb};
        return ret;
    }

    Handle NewVector(std::size_t items) {
        void* addr = Allocate(Vector::AllocationSize(items));
        Vector* ptr = new (addr) Vector(items);
        std::shared_ptr<HandleBlock> hb = std::make_shared<HandleBlock>(&roots, Reference(ptr));
        Handle ret{hb};
        return ret;
    }

    Handle NewString(const std::string& str) {
        void* addr = Allocate(String::AllocationSize(str));
        String* ptr = new (addr) String(str);
        std::shared_ptr<HandleBlock> hb = std::make_shared<HandleBlock>(&roots, Reference(ptr));
        Handle ret{hb};
        return ret;
    }

    Handle NewPair(Handle first, Handle second) {
        return StructureAllocator<Pair>(first, second);
    }

    Handle NewMap() {
        return StructureAllocator<Map>();
    }

    Handle NewEnvironment(Handle outer, Handle lookup) {
        return StructureAllocator<Envrionment>(outer, lookup);
    }

    Handle NewStack() {
        return StructureAllocator<Stack>();
    }

    Handle NewFrame(Handle bytecode, Handle outer, Handle temps, Handle env) {
        return StructureAllocator<Frame>(bytecode, outer, temps, env);
    }

    Handle NewNativeFunction(Handle ptr, Handle arity) {
        return StructureAllocator<NativeFunction>(ptr, arity);
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

            void OnNil(const Nil*) override {/* intentionally empty */}
            void OnInteger(const Integer*) override {/* intentionally empty */}
            void OnSymbol(const Symbol*) override {/* intentionally empty */}
            void OnReal(const Real*) override {/* intentionally empty */}
            void OnBoolean(const Boolean*) override {/* intentionally empty */}
            void OnNativeReference(const NativeReference*) override {/* intentionally empty */}
            void OnCharacter(const Character*) override {/* intentionally empty */}
            void OnReference(const Reference* ref) override {
                DEBUGLN("Reference object detected, moving to new space");
                heap->transferReference(location);
            }
        } visitor(this, location);

        location->Visit(visitor);
    }

    void transferReference(Primitive* location) {
        Object* ref = location->AsReference()->Value();

        // if it's already been moved, just update the location with the
        // new pointer
        if (ref->IsGcForward()) {
            DEBUGLN("Gc forward detected, " << ref << " was already moved moved to " << ref->GetGcForwardAddress());
            *location = Reference(ref->GetGcForwardAddress());
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
        *location = Reference(new_addr_casted);
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

#endif // HEAP_H__