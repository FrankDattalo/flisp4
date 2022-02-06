#ifndef FILEREGISTRY_HH__
#define FILEREGISTRY_HH__

#include <map>
#include <string>
#include <memory>
#include <array>
#include <mutex>
#include <optional>

#include "util/memory_semantic_macros.hh"

#include "bytecode/bytecode.hh"

namespace runtime {
class RegistryBlock {
private:
    constexpr static std::size_t BLOCK_SIZE = 4096;
    std::unique_ptr<RegistryBlock> outer;
    std::array<bytecode::File, BLOCK_SIZE> block;
    std::size_t first_free;

public:
    RegistryBlock(std::unique_ptr<RegistryBlock> _outer)
    : outer{std::move(_outer)}
    {}

    ~RegistryBlock() = default;

    NOT_COPYABLE(RegistryBlock);
    MOVEABLE(RegistryBlock);

    bool IsFull() const {
        return block.size() == first_free;
    }

    const bytecode::File* Register(bytecode::File file) {
        block.at(first_free) = std::move(file);
        const bytecode::File* result = &block.at(first_free);
        first_free++;
        return result;
    }
};

class FileRegistry {
private:
    std::unique_ptr<RegistryBlock> block;
    std::map<std::string, const bytecode::File*> lookup;
    std::mutex mutex;
public:
    FileRegistry() = default;
    ~FileRegistry() = default;

    NOT_COPYABLE(FileRegistry);
    NOT_MOVEABLE(FileRegistry);

    std::optional<const bytecode::File*> Lookup(const std::string& name) {
        std::scoped_lock lock{mutex};

        auto result = lookup.find(name);

        if (result == lookup.end()) {
            return std::nullopt;
        }

        return result->second;
    }

    std::optional<const bytecode::Function*> LookupFunction(const std::string& module, const std::string& function) {
        auto moduleLookup = Lookup(module);
        if (moduleLookup == std::nullopt) {
            return std::nullopt;
        }
        const bytecode::File* file = *moduleLookup;
        // TODO: remove linear search
        for (const bytecode::Function& fn: file->GetFunctions()) {
            if (fn.GetName() == function) {
                return &fn;
            }
        }
        return std::nullopt;
    }

    const bytecode::File* Register(bytecode::File file) {
        std::scoped_lock lock{mutex};

        if (lookup.find(file.GetModuleName()) != lookup.end()) {
            std::string msg{"Redefinition of module: "};
            msg.append(file.GetModuleName());
            throw std::runtime_error{msg};
        }

        const bytecode::File* result = block->Register(std::move(file));
        lookup[result->GetModuleName()] = result;

        return result;
    }
};
}

#endif // FILEREGISTRY_HH__