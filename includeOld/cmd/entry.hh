#ifndef ENTRY_HH__
#define ENTRY_HH__

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <system_error>

#include "decompile.hh"
#include "execute.hh"
#include "assemble.hh"

#include "refactor/object.hh"

namespace cmd {

std::vector<std::string> getArgs(int argc, char** argv) {
    std::vector<std::string> result;
    result.reserve(argc);
    for (int i = 0; i < argc; i++) {
        result.emplace_back(argv[i], strlen(argv[i]));
    }
    return result;
}

using CommandFunction = void(*)(const std::vector<std::string>&);

CommandFunction selectCommandEntry(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        throw std::runtime_error{std::string{"No arguments given, at least one is expected"}};
    }
    const std::string& subcommand = args.at(1);
    if (subcommand == "decompile") {
        return decompile;
    }
    if (subcommand == "execute") {
        return execute;
    }
    if (subcommand == "assemble") {
        return assemble;
    }
    std::string message{"Unknown command: "};
    message.append(subcommand);
    throw std::runtime_error{message};
}

void printArgs(const std::vector<std::string>& args) {
    DEBUGLN("Arguments:");
    for (std::size_t i = 0; i < args.size(); i++) {
        DEBUGLN("[" << i << "] " << args.at(i));
    }
}

int entry(int argc, char** argv) {
    try {
        std::vector<std::string> args = getArgs(argc, argv);
        if (IS_DEBUG_ENABLED()) {
            printArgs(args);
        }
        CommandFunction fn = selectCommandEntry(args);
        fn(args);
        return 0;
    } catch (const std::system_error& e) {
        std::cerr << "System error: " << e.code() << " " << e.what() << std::endl;
        return 1;
    } catch (const std::exception & e) {
        std::cerr << "Uncaught error: " << e.what() << std::endl;
        return 1;
    }
}

}

#endif // ENTRY_H__