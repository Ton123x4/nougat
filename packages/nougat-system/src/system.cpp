#define STDEXT_LIB_IMPL

#include "system.hpp"
#include "defines.h"
#include <stdext_sys.h>
#include <stdext/handler.hpp>
#include <stdext/memory.hpp>
#include <stdext/logger.hpp>
#include <stdext/filesystem.hpp>
#include <stdext/system.hpp>
#include <format>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

static std::string nougatEnvironment;
static std::string nougatBinaries;
static std::string nougatLibraries;
static std::string nougatPackages;
static std::string nougatRegistry;
static std::string nougatSymbols;
static std::string nougatBinary;


void InitNougatSystem() {
    auto nougatHome = stdext::system::get_env(NOUGAT_HOME_PATH);

    if (nougatHome == nullptr) {
        throw std::runtime_error(std::format("Failed to initialize nougat system. {} is not defined.", NOUGAT_HOME_PATH));
    }

    auto nougatHomePath = stdext::fs::path(nougatHome);
    auto nougatStorePath = stdext::fs::join_path(nougatHome, NOUGAT_REGISTRY);

    stdext::debug("Initializing nougat system path...");

    nougatBinaries = stdext::fs::absolute(nougatHomePath / NOUGAT_BINARIES);
    nougatEnvironment = stdext::fs::absolute(nougatHomePath / NOUGAT_ENVIRONMENT);
    nougatPackages = stdext::fs::absolute(nougatHomePath / NOUGAT_PACKAGES);
    nougatRegistry = stdext::fs::absolute(nougatHomePath / NOUGAT_PACKAGES_DB);
    nougatSymbols = stdext::fs::absolute(nougatHomePath / NOUGAT_SYMBOLS_DB);
    nougatBinary = stdext::fs::absolute(nougatHomePath / NOUGAT_SHIM_EXE);

    stdext::debug("Creating nougat system directories...");

    if (!stdext::fs::exists(nougatBinaries)) {
        stdext::debug("Creating binaries directory: {}", nougatBinaries);
        stdext::fs::create_directories(nougatBinaries);
    }

    if (!stdext::fs::exists(nougatEnvironment)) {
        stdext::debug("Creating environment directory: {}", nougatEnvironment);
        stdext::fs::create_directories(nougatEnvironment);
    }

    if (!stdext::fs::exists(nougatPackages)) {
        stdext::debug("Creating packages directory: {}", nougatPackages);
        stdext::fs::create_directories(nougatPackages);
    }

    if (!stdext::fs::exists(nougatStorePath)) {
        stdext::debug("Creating store directory: {}", nougatStorePath);
        stdext::fs::create_directories(nougatStorePath);
    }

    stdext::debug("Nougat system directories created.");
}


const std::string& GetNougatBinaries() {
    return nougatBinaries;
}

const std::string& GetNougatPackages() {
    return nougatPackages;
}

const std::string& GetNougatBinary() {
    return nougatBinary;
}


std::string GetBinaryName(const std::string& filename) {
    auto directoryPath = stdext::fs::join_path(nougatBinaries, filename);
    auto binaryName = stdext::fs::absolute(directoryPath);

    return std::format("{}{}", binaryName, STDEXT_EXECUTABLE_EXT);
}

std::string GetPackageID(const std::string& filename) {
    auto registry = stdext::make_safe(OpenNougatRegistry(), CloseDatabase);
    auto identifier = stdext::db::get(registry.get(), filename);

    if (!identifier) {
        throw std::runtime_error("Package ID not found.");
    }

    return std::string(identifier);
}

std::string GetPackagePath(const std::string& packageId) {
    auto directoryPath = stdext::fs::join_path(nougatPackages, packageId);
    auto packagePath = stdext::fs::absolute(directoryPath);

    return packagePath;
}

std::string GetEnvironmentPath(const std::string& packageId) {
    auto directoryPath = stdext::fs::join_path(nougatEnvironment, packageId);
    auto packagePath = stdext::fs::absolute(directoryPath);

    return std::format("{}.db", packagePath);
}

std::string GetExecutablePath(const std::string& filename) {
    auto symbols = stdext::make_safe(OpenNougatSymbols(), CloseDatabase);
    auto filepath = stdext::db::get(symbols.get(), filename);

    if (!filepath) {
        throw std::runtime_error("Executable reference not found.");
    }

    return std::string(filepath);
}



stdext::database* OpenPackageEnvironment(const std::string& identifier) {
    auto filepath = GetEnvironmentPath(identifier);
    auto database = stdext::db::open(filepath);

    if (database == nullptr) {
        throw std::runtime_error(std::format("Failed to open package environment: {}", filepath));
    }

    return database;
}

stdext::database* OpenNougatRegistry() {
    auto database = stdext::db::open(nougatRegistry);

    if (database == nullptr) {
        throw std::runtime_error(std::format("Failed to open nougat registry: {}", nougatRegistry));
    }

    return database;
}

stdext::database* OpenNougatSymbols() {
    auto database = stdext::db::open(nougatSymbols);

    if (database == nullptr) {
        throw std::runtime_error(std::format("Failed to open nougat executables: {}", nougatSymbols));
    }

    return database;
}

void CloseDatabase(stdext::database* db) {
    stdext::db::close(db);
}



int ExecuteApplication(const std::string& identifier, const std::vector<std::string>& arguments) {
    auto executable = GetExecutablePath(identifier);
    auto environment = OpenPackageEnvironment(identifier);
    auto variables = stdext::db::list(environment);

    stdext::debug("Executable: {}", executable);

    for (auto& variable : variables) {
        stdext::system::set_env(variable, stdext::db::get(environment, variable));
    }

    CloseDatabase(environment);

    return stdext::system::wait(stdext::system::fork(executable, arguments));
}
