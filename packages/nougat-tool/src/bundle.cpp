#include "bundle.hpp"
#include "system.hpp"
#include "defines.h"
#include <stdext/memory.hpp>
#include <stdext/string.hpp>
#include <stdext/database.hpp>
#include <stdext/filesystem.hpp>
#include <stdext/fstream.hpp>
#include <stdext/logger.hpp>
#include <filesystem>
#include <format>
#include <string>
#include <unordered_set>

#ifdef _WIN32
const std::unordered_set<std::string> kExecutableExt {
    ".exe",
    ".com",
    ".cmd",
    ".bat",
    ".ps1"
};
#endif

const std::unordered_set<std::string> kSearchPath {
    "./",
    "./bin",
    "./binaries",
    "./toolchain",

    #ifndef _WIN32
    "./Binaries",
    "./Toolchain"
    #endif
};


void CreateExecutableLink(const std::string& shimBinaryPath, const std::string& filename) {
    auto filepath = GetBinaryName(filename);

    if (stdext::fs::exists(filepath)) {
        return;
    }

    stdext::fs::copy_file(shimBinaryPath, filepath);
}

void SetupPackage(
    const std::string& packagePath,
    const std::string& binariesPath,
    const std::string& identifier
) {
    auto nougatBinary = GetNougatBinary();

    auto registry = stdext::make_safe(OpenNougatRegistry(), CloseDatabase);
    auto executables = stdext::make_safe(OpenNougatSymbols(), CloseDatabase);
    auto envDatabase = stdext::make_safe(OpenPackageEnvironment(identifier), CloseDatabase);

    auto searchPath = std::unordered_set<std::string>(kSearchPath);

    if (!binariesPath.empty()) {
        searchPath.insert(binariesPath);
    }

    for (const auto& searchDirectory : searchPath) {
        auto directoryPath = stdext::fs::absolute(stdext::fs::join_path(packagePath, searchDirectory));

        if (!stdext::fs::exists(directoryPath)) {
            stdext::debug("Search path not found, skipping: {}", directoryPath);
            continue;
        }

        stdext::debug("Scanning directory: {}", directoryPath);

        for (const auto& entry : stdext::fs::directory_iterator(directoryPath)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            auto entryPath = entry.path();
            auto fileName = entryPath.stem();
            auto fileExtension = entryPath.extension();

#ifdef _WIN32
            if (!kExecutableExt.contains(fileExtension.string())) {
                stdext::debug("Skipping non-executable: {}", fileName.string());
                continue;
            }
#endif

            stdext::debug("Registering executable: {}", fileName.string());

            stdext::db::set(executables.get(), fileName.string(), entryPath.string());
            stdext::db::set(registry.get(), fileName.string(), identifier);

            CreateExecutableLink(nougatBinary, fileName.string());
        }
    }

    stdext::db::set(envDatabase.get(), NOUGAT_PACKAGE_HOME, packagePath);
    stdext::db::set(envDatabase.get(), NOUGAT_PACKAGE_ID, identifier);

    stdext::debug("Package home: {}", packagePath);
    stdext::debug("Package id: {}", identifier);
}
