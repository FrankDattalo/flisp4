#ifndef NATIVE_REGISTRY_HH__
#define NATIVE_REGISTRY_HH__

#include <string>
#include <map>
#include <stdexcept>

#include "debug.hh"

#include "memory_semantic_macros.hh"

class VirtualMachine;

using NativeFunctionHandle = void (*)(VirtualMachine* vm);

class NativeFunction {
private:
    std::string name;
    std::uint64_t arity;
    NativeFunctionHandle handle;
public:
    NativeFunction(std::string _name, std::uint64_t _arity, NativeFunctionHandle _handle)
    : name{std::move(_name)}, arity{_arity}, handle{_handle}
    {}

    ~NativeFunction() = default;

    NOT_COPYABLE(NativeFunction);
    MOVEABLE(NativeFunction);

    void Apply(VirtualMachine* vm) const {
        handle(vm);
    }

    const std::string& GetName() const {
        return this->name;
    }

    const std::uint64_t GetArity() const {
        return this->arity;
    }
};

class NativeFunctionRegistry {
private:
    std::map<std::string, NativeFunction> fns;
public:
    NativeFunctionRegistry() = default;
    ~NativeFunctionRegistry() = default;
    NOT_COPYABLE(NativeFunctionRegistry);
    NOT_MOVEABLE(NativeFunctionRegistry);

    const NativeFunction& Get(const std::string & name) const {
        DEBUGLN("Load native " << name);
        auto iter = fns.find(name);
        if (iter == fns.end()) {
            std::string msg{"No native function defined with name: "};
            msg.append(name);
            throw std::runtime_error{msg};
        }
        return iter->second;
    }

    void Register(NativeFunction native_fn) {
        DEBUGLN("Register native " << native_fn.GetName());
        std::string name = native_fn.GetName();
        this->fns.insert({name, std::move(native_fn)});
    }
};

#endif // NATIVE_REGISTRY_HH__