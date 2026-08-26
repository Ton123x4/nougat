#include "commands.hpp"
#include "system.hpp"
#include <stdext/string.hpp>
#include <stdext/forge.hpp>
#include <stdext/console.hpp>
#include <stdext/arg_parser.hpp>
#include <stdext/filesystem.hpp>
#include <cstdlib>
#include <exception>
#include <stdexcept>

static const char kHelpMessage[] = {
    #embed "./resources/help-tool.txt"
    , '\0'
};

void HelpMessage() {
    stdext::println("{}", kHelpMessage);
}

int main(int argc, char* argv[]) {
    stdext::init_console();

    if (argc == 1) {
        HelpMessage();
        return EXIT_SUCCESS;
    }

    try {
        auto arg_parser = stdext::arg_parser(argc, argv);

        InitNougatSystem();

        if (arg_parser.match({ "--version", "-v" })) {
            stdext::forge::print_build_info("Nougat Package Manager");
        }
        else if (arg_parser.match({ "--help", "-h" })) {
            HelpMessage();
        }
        else if (arg_parser.match({ "install" })) {
            HandleInstall(arg_parser);
        }
        else if (arg_parser.match({ "repair" })) {
            HandleRepair(arg_parser);
        }
        else if (arg_parser.match({ "uninstall" })) {
            HandleUninstall(arg_parser);
        }
        else if (arg_parser.match({ "commands" })) {
            HandleCommands(arg_parser);
        }
        else if (arg_parser.match({ "list" })) {
            HandleList(arg_parser);
        }
        else {
            throw std::runtime_error(std::format("Unknown command: {}", arg_parser.next()));
        }
    } catch (const std::exception& error) {
        stdext::println(stdext::ansi_color::red, "Error: {}", error.what());
        return EXIT_FAILURE;
    }
}
