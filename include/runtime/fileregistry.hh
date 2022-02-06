#ifndef FILEREGISTRY_HH__
#define FILEREGISTRY_HH__

#include <map>
#include <string>
#include <memory>
#include <array>
#include <mutex>
#include <optional>
#include <utility>

#include "util/memory_semantic_macros.hh"

#include "bytecode/bytecode.hh"

namespace runtime {
template <typename T, std::size_t SIZE>
class RegistryBlock {
private:
    std::unique_ptr<RegistryBlock<T, SIZE>> outer;
    std::array<T, SIZE> block;
    std::size_t first_free;

public:
    RegistryBlock(std::unique_ptr<RegistryBlock<T, SIZE>> _outer)
    : outer{std::move(_outer)}, first_free{0}
    {}

    ~RegistryBlock() = default;

    NOT_COPYABLE(RegistryBlock);
    NOT_MOVEABLE(RegistryBlock);

    bool IsFull() const {
        return block.size() == first_free;
    }

    const T* Register(T file) {
        block.at(first_free) = std::move(file);
        const T* result = &block.at(first_free);
        first_free++;
        return result;
    }
};

template <typename T, std::size_t SIZE>
class RegistryList {
private:
    std::unique_ptr<RegistryBlock<T, SIZE>> head;
public:
    RegistryList() = default;

    ~RegistryList() = default;

    NOT_COPYABLE(RegistryList);
    NOT_MOVEABLE(RegistryList);

    const T* Register(T data) {
        if (head->IsFull()) {
            auto temp = std::move(head);
            head = std::make_unique<RegistryBlock<T, SIZE>>(std::move(temp));
        }
        return head->Register(std::move(data));
    }
};

class FileRegistry {
private:
    using function_key = std::pair<std::string, std::string>;

    constexpr static std::size_t BLOCK_SIZE = 1;
    RegistryList<bytecode::File, BLOCK_SIZE> files;
    std::map<std::string, const bytecode::File*> file_lookup;
    std::map<function_key, const bytecode::Function*> function_lookup;
    std::mutex mutex;
public:
    FileRegistry() = default;
    ~FileRegistry() = default;

    NOT_COPYABLE(FileRegistry);
    NOT_MOVEABLE(FileRegistry);

    std::optional<const bytecode::File*> Lookup(const std::string& name) {

        std::scoped_lock lock{mutex};

        auto result = file_lookup.find(name);
        if (result == file_lookup.end()) {
            return std::nullopt;
        }

        return result->second;
    }

    std::optional<const bytecode::Function*> LookupFunction(const std::string& module, 
                                                            const std::string& function) {

        std::scoped_lock lock{mutex};

        function_key key = std::make_pair(module, function);
        auto result = function_lookup.find(key);
        if (result == function_lookup.end()) {
            return std::nullopt;
        }

        return result->second;
    }

    const bytecode::File* Register(bytecode::File file) {

        std::scoped_lock lock{mutex};

        if (file_lookup.find(file.GetModuleName()) != file_lookup.end()) {
            std::string msg{"Redefinition of module: "};
            msg.append(file.GetModuleName());
            throw std::runtime_error{msg};
        }

        const bytecode::File* result = files.Register(std::move(file));
        file_lookup[result->GetModuleName()] = result;

        for (const bytecode::Function& fn : result->GetFunctions()) {
            function_key key = std::make_pair(file.GetModuleName(), fn.GetName());
            function_lookup[key] = &fn;
        }

        return result;
    }
};
}

#endif // FILEREGISTRY_HH__