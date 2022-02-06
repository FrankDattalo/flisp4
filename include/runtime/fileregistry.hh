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

template<typename T, std::size_t BLOCK_SIZE>
class IndexedRegistry {
private:
    RegistryList<T, BLOCK_SIZE> list;
    std::map<typename T::Key, const T*> lookup;
public:
    IndexedRegistry() = default;
    ~IndexedRegistry() = default;

    NOT_COPYABLE(IndexedRegistry);
    NOT_MOVEABLE(IndexedRegistry);

    std::optional<const T*> Lookup(const typename T::Key& key) {
        auto result = lookup.find(key);
        if (result == lookup.end()) {
            return std::nullopt;
        }
        return result->second;
    }

    const T* Register(T data) {
        auto key = data.GetKey();
        if (lookup.find(key) != lookup.end()) {
            throw std::runtime_error{"Redefinition error"};
        }
        const T* result = list.Register(std::move(data));
        lookup[key] = result;
        return result;
    }
};

class RegisteredFile {
private:
    bytecode::File file;
public:
    using Key = std::string;

    RegisteredFile() = default;

    RegisteredFile(bytecode::File _file)
    : file{std::move(_file)}
    {}

    ~RegisteredFile() = default;

    NOT_COPYABLE(RegisteredFile);
    MOVEABLE(RegisteredFile);

    Key GetKey() const {
        return file.GetModuleName();
    }

    const bytecode::File* GetFile() const { return &file; }
};

class RegisteredFunction {
private:
    const bytecode::Function* function;
    const RegisteredFile* file;
public:
    using Key = std::pair<std::string, std::string>;

    RegisteredFunction() = default;

    RegisteredFunction(const bytecode::Function* _function, const RegisteredFile* _file)
    : function{_function}, file{_file}
    {}

    ~RegisteredFunction() = default;

    NOT_COPYABLE(RegisteredFunction);
    MOVEABLE(RegisteredFunction);

    Key GetKey() const {
        return std::make_pair(
            file->GetKey(),
            function->GetName()
        );
    }

    const bytecode::Function* GetFunction() const { return function; }
    const bytecode::File* GetFile() const { return file->GetFile(); }
};

class FileRegistry {
private:
    constexpr static std::size_t FILE_BLOCK_SIZE = 1;
    constexpr static std::size_t FUNCTION_BLOCK_SIZE = 20;

    using Files = IndexedRegistry<RegisteredFile, FILE_BLOCK_SIZE>;
    using Functions = IndexedRegistry<RegisteredFunction, FUNCTION_BLOCK_SIZE>;

    Files files;
    Functions functions;

    std::mutex mutex;
public:
    FileRegistry() = default;
    ~FileRegistry() = default;

    NOT_COPYABLE(FileRegistry);
    NOT_MOVEABLE(FileRegistry);

    std::optional<const RegisteredFile*> LookupFile(const std::string& name) {
        std::scoped_lock lock{mutex};
        return files.Lookup(name);
    }

    std::optional<const RegisteredFunction*> LookupFunction(const std::string& module, 
                                                            const std::string& function) {
        std::scoped_lock lock{mutex};
        RegisteredFunction::Key key = std::make_pair(module, function);
        return functions.Lookup(key);
    }

    const RegisteredFile* Register(bytecode::File file) {

        std::scoped_lock lock{mutex};

        RegisteredFile reg_file{std::move(file)};
        const RegisteredFile* result = files.Register(std::move(reg_file));

        for (const bytecode::Function& fn : result->GetFile()->GetFunctions()) {
            RegisteredFunction reg_fn{&fn, result};
            functions.Register(std::move(reg_fn));
        }

        return result;
    }
};
}

#endif // FILEREGISTRY_HH__