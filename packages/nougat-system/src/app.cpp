#define STDEXT_LIB_IMPL

#define STDEXT_INFO_TITLE    "[Nougat]"
#define STDEXT_ERROR_TITLE   "[Nougat]"
#define STDEXT_SUCCESS_TITLE "[Nougat]"

#include "app.h"
#include "system.hpp"

#include <stdext/arg_parser.hpp>
#include <stdext/console.hpp>
#include <stdext/logger.hpp>
#include <stdext/filesystem.hpp>
#include <stdext/string.hpp>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <cstring>

static const char helpMessage[] = {
    #embed "./resources/help-app.txt"
    , '\0'
};

int HelpMessage(const std::string& filename) {
    stdext::println("{}", stdext::string::replace(helpMessage, "{executable}", filename));
    return EXIT_SUCCESS;
}

int SetVariable(const std::string& filename, stdext::arg_parser& parser) {
    auto environment = OpenPackageEnvironment(filename);
    auto variableName = parser.expect("Missing variable name");
    auto variableValue = parser.expect("Missing variable value");

    stdext::db::set(environment, variableName, variableValue);
    stdext::db::close(environment);

    stdext::success("Variable successfully updated!");
    return EXIT_SUCCESS;
}

int GetVariable(const std::string& filename, stdext::arg_parser& parser) {
    auto environment = OpenPackageEnvironment(filename);
    auto variableName = parser.expect("Missing variable name");
    auto variableValue = stdext::db::get(environment, variableName);

    if (variableValue) {
        stdext::println(stdext::ansi_color::bright_yellow, "{}", variableValue);
    } else {
        throw std::runtime_error(std::format("Variable does not exists: {}", variableName));
    }

    stdext::db::close(environment);

    return EXIT_SUCCESS;
}

int UnsetVariable(const std::string& filename, stdext::arg_parser& parser) {
    auto variableName = parser.expect("Missing variable name");
    auto environment = OpenPackageEnvironment(filename);

    stdext::db::remove(environment, variableName);
    stdext::db::close(environment);
    return EXIT_SUCCESS;
}

int ListVariable(const std::string& filename, stdext::arg_parser& parser) {
    auto environment = OpenPackageEnvironment(filename);

    for (auto& variable : stdext::db::list(environment)) {
        auto variableName = stdext::fg_color(stdext::ansi_color::bright_blue, variable);
        auto variableValue = stdext::fg_color(stdext::ansi_color::bright_yellow, stdext::db::get(environment, variable));

        stdext::println("{} = {}", variableName, variableValue);
    }

    stdext::db::close(environment);
    return EXIT_SUCCESS;
}

int PrintPackageId(const std::string& filename) {
    stdext::println(stdext::ansi_color::bright_yellow, "{}", GetPackageID(filename));
    return EXIT_SUCCESS;
}


int PrintPackageWhere(const std::string& filename) {
    stdext::println(stdext::ansi_color::bright_yellow, "{}", GetExecutablePath(filename));
    return EXIT_SUCCESS;
}

int LaunchApplication(const std::string& filename, stdext::arg_parser& parser) {
    auto arguments = std::vector<std::string>();

    InitNougatSystem();

    while (!parser.is_end()) {
        auto command = parser.next();

        if (!strcmp(command, "--nougat:help")) {
            return HelpMessage(filename);
        }

        if (!strcmp(command, "--nougat:set")) {
            return SetVariable(filename, parser);
        }

        if (!strcmp(command, "--nougat:get")) {
            return GetVariable(filename, parser);;
        }

        if (!strcmp(command, "--nougat:unset")) {
            return UnsetVariable(filename, parser);
        }

        if (!strcmp(command, "--nougat:list")) {
            return ListVariable(filename, parser);
        }

        if (!strcmp(command, "--nougat:id")) {
            return PrintPackageId(filename);
        }

        if (!strcmp(command, "--nougat:where")) {
            return PrintPackageWhere(filename);
        }

        if (stdext::string::starts_with(command, "--nougat:")) {
            throw std::runtime_error(std::format("Unknown internal command {}", command));
        }

        arguments.push_back(command);
    }

    return ExecuteApplication(filename, arguments);
}

int LaunchApplication(int argc, char* argv[]) {
    auto filename = stdext::fs::filename(argv[0]);
    auto parser = stdext::arg_parser(argc, argv);

    stdext::init_console();

    try {
        return LaunchApplication(filename, parser);
    } catch (const std::exception& error) {
        stdext::error("Unexpected error: {}", error.what());
        return EXIT_FAILURE;
    }
}
