#ifndef SYMBOL_TABLE_H__
#define SYMBOL_TABLE_H__

#include <cstdint>
#include <map>
#include <mutex>

#include "util/memory_semantic_macros.hh"

namespace runtime {

class SymbolTable {
private:
    std::map<std::string, std::uint64_t> symbols_to_id;
    std::map<std::uint64_t, std::string> id_to_symbols;
    std::mutex mutex;
public:
    SymbolTable() {}

    ~SymbolTable() = default;

    NOT_COPYABLE(SymbolTable);

    NOT_MOVEABLE(SymbolTable);

    std::uint64_t Intern(const std::string & value) {

        std::scoped_lock lock{mutex};

        auto iter = this->symbols_to_id.find(value);

        if (iter != this->symbols_to_id.end()) {
            return iter->second;
        }

        std::uint64_t result = static_cast<std::uint64_t>(this->symbols_to_id.size());

        this->symbols_to_id[value] = result;
        this->id_to_symbols[result] = value;

        return result;
    }

    std::string ToString(std::uint64_t symbol_id) {

        std::scoped_lock lock{mutex};

        auto iter = this->id_to_symbols.find(symbol_id);

        if (iter == this->id_to_symbols.end()) {
            std::string error_message{"No symbol definition found for "};
            error_message.append(std::to_string(symbol_id));
            throw std::runtime_error{error_message};
        }

        return iter->second;
    }
};

}

#endif // SYMBOL_TABLE_H__