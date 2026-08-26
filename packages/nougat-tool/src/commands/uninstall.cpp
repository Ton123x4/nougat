#include "commands.hpp"
#include "system.hpp"
#include "utils.hpp"
#include <stdext/memory.hpp>
#include <stdext/filesystem.hpp>
#include <stdext/logger.hpp>
#include <string.h>

void HandleUninstall(stdext::arg_parser& argParser) {
    auto packageId = argParser.next();
    auto binariesPath = GetNougatBinaries();
    auto packagePath = GetPackagePath(packageId);
    auto packageEnv = GetEnvironmentPath(packageId);

    auto globalRegistry = stdext::make_safe(OpenNougatRegistry(), CloseDatabase);
    auto globalSymbols = stdext::make_safe(OpenNougatSymbols(), CloseDatabase);

    stdext::debug("Package path: {}", packagePath);
    stdext::log("Uninstalling '{}'...", packageId);

    auto executables = std::vector<std::string>();

    for (auto& filename : stdext::db::list(globalRegistry.get())) {
        auto identifier = stdext::db::get(globalRegistry.get(), filename);

        stdext::debug("{}: {}", identifier, filename);

        if (!strcmp(identifier, packageId)) {
            executables.push_back(filename);
        }
    }

    if (executables.empty()) {
        stdext::log("No registered executables found for '{}'", packageId);
    }
    else {
        stdext::log("Removing {} executable(s)...", executables.size());

        for (auto& filename : executables) {
            auto filepath = stdext::fs::join_path(binariesPath, filename);
            auto fullpath = stdext::fs::absolute(filepath);

            stdext::debug("Removing shim '{}'", fullpath);
            stdext::db::remove(globalRegistry.get(), filename);
            stdext::db::remove(globalSymbols.get(), filename);
            stdext::fs::remove(fullpath);
        }
    }

    stdext::info("Removing package environment database...");
    stdext::debug("Removing: {}", packageEnv);
    stdext::fs::remove(packageEnv);

    stdext::info("Removing package directory...");
    stdext::debug("Removing: {}", packagePath);

    RemovePackageDirectory(packagePath);

    stdext::log("Package '{}' uninstalled.", packageId);
}
