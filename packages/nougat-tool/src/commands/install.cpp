#include "commands.hpp"
#include "bundle.hpp"
#include "utils.hpp"
#include "system.hpp"
#include <stdext/string.hpp>
#include <stdext/filesystem.hpp>
#include <stdext/fstream.hpp>
#include <stdext/logger.hpp>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

void InstallLocalPackage(
    const std::string& packageFile,
    const std::string& binariesPath,
    const std::string& identifier
) {
    auto packagePath = GetPackagePath(identifier);

    stdext::debug("Install path: {}", packagePath);
    stdext::debug("Package file: {}", packageFile);
    stdext::debug("Binaries path: {}", binariesPath.empty() ? "(none)" : binariesPath);

    if (fs::exists(packagePath)) {
        stdext::debug("Package '{}' already exists at '{}'", identifier, packagePath);

        if (!stdext::confirm("Package '{}' is already installed. Reinstall?", identifier)) {
            stdext::error("Installation cancelled.");
            return;
        }

        stdext::warn("Removing existing package: {}", identifier);
        RemovePackageDirectory(packagePath);
        stdext::debug("Removed existing package directory.");
    }

    if (fs::is_directory(packageFile)) {
        stdext::log("Linking '{}' as package '{}'...", packageFile, identifier);
        stdext::debug("Creating symlink: {} ==> {}", packageFile, packagePath);
        CreatePackageLink(packageFile, packagePath);
        stdext::debug("Symlink created successfully.");
    } else {
        stdext::log("Installing '{}'...", identifier);
        stdext::debug("Extracting '{}' to '{}'...", packageFile, packagePath);
        ExtractPackage(packageFile, packagePath);
        stdext::debug("Extraction complete.");
    }

    stdext::log("Setting package up...");
    SetupPackage(packagePath, binariesPath, identifier);

    stdext::success("Package '{}' successfully installed!", identifier);
}

void InstallExternalPackage(
    const std::string& packageUri,
    const std::string& binariesPath,
    const std::string& identifier
) {
    auto tempDirectory = fs::temp_directory_path();
    auto tempPath = tempDirectory / std::format("{}.zip", identifier);

    stdext::debug("Temp directory: {}", tempDirectory.string());
    stdext::debug("Temp file: {}", tempPath.generic_string());

    stdext::log("Downloading '{}'...", identifier);
    stdext::debug("Source URL: {}", packageUri);
    DownloadPackage(packageUri, tempPath.generic_string());
    stdext::debug("Download complete: {}", tempPath.generic_string());

    stdext::debug("Handing off to local installer...");
    InstallLocalPackage(tempPath.generic_string(), binariesPath, identifier);

    stdext::debug("Removing temporary file: {}", tempPath.string());
    stdext::fs::remove(tempPath.string());
    stdext::debug("Temporary file removed.");
}

void InstallPackage(
    const std::string& packageUri,
    const std::string& binariesPath,
    const std::string& identifier
) {
    if (packageUri.starts_with("http")) {
        stdext::debug("URI scheme detected as remote, using external installer.");
        return InstallExternalPackage(packageUri, binariesPath, identifier);
    }

    stdext::debug("URI detected as local path, using local installer.");
    return InstallLocalPackage(packageUri, binariesPath, identifier);
}

void HandleInstall(stdext::arg_parser& argParser) {
    auto packageUri = std::string();
    auto identifier = std::string();
    auto binariesPath = std::string();

    while (!argParser.is_end()) {
        if (argParser.match({ "--id" })) {
            identifier = argParser.expect("Expected a value after '--id'.");
            stdext::debug("Flag --id: '{}'", identifier);
        }
        else if (argParser.match({ "--bin" })) {
            binariesPath = argParser.expect("Expected a value after '--bin'.");
            stdext::debug("Flag --bin: '{}'", binariesPath);
        }
        else {
            auto argument = argParser.next();

            if (!packageUri.empty()) {
                throw std::runtime_error(std::format("Unexpected argument: '{}'", argument));
            }

            packageUri = argument;
            stdext::debug("Package URI set to: '{}'", packageUri);
        }
    }

    if (packageUri.empty()) {
        throw std::runtime_error("No package specified.");
    }

    if (packageUri.ends_with("\\")) {
        packageUri.pop_back();
    }

    if (packageUri.ends_with("/")) {
        packageUri.pop_back();
    }

    if (identifier.empty()) {
        identifier = stdext::fs::stem(packageUri);
        stdext::debug("No --id provided, inferred identifier from filename: '{}'", identifier);
    }

    stdext::debug("Resolved package URI: '{}'", packageUri);
    stdext::debug("Resolved identifier: '{}'", identifier);
    stdext::debug("Resolved binaries path: '{}'", binariesPath.empty() ? "(none)" : binariesPath);

    InstallPackage(packageUri, binariesPath, identifier);
}
