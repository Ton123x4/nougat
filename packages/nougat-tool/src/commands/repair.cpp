#include "bundle.hpp"
#include "commands.hpp"
#include "system.hpp"
#include "stdext/logger.hpp"
#include <filesystem>

void RepairPackages() {
    auto packagesPath = GetNougatPackages();

    if (!std::filesystem::exists(packagesPath)) {
        stdext::info("No packages installed.");
        return;
    }

    int count = 0;

    for (const auto& entry : std::filesystem::directory_iterator(packagesPath)) {
        if (!entry.is_directory()) {
            continue;
        }

        auto packageId = entry.path().filename().string();
        auto packagePath = entry.path().string();

        stdext::log("Repairing '{}'...", packageId);

        try {
            SetupPackage(packagePath, {}, packageId);
            count++;
        }
        catch (const std::exception& error) {
            stdext::error("Failed to repair '{}': {}", packageId, error.what());
        }
    }

    stdext::success("Repaired {} package(s).", count);
}

void HandleRepair(stdext::arg_parser& argParser) {
    auto packageId = argParser.optional();

    if (packageId == nullptr) {
        stdext::debug("No package specified, repairing all packages...");
        RepairPackages();
        return;
    }

    auto customBin = argParser.optional();
    auto packagePath = GetPackagePath(packageId);

    stdext::debug("Package path: {}", packagePath);

    if (!std::filesystem::exists(packagePath)) {
        stdext::error("Package '{}' is not installed.", packageId);
        return;
    }

    auto binariesPath = std::string();

    if (customBin != nullptr) {
        binariesPath = customBin;
    }

    stdext::log("Repairing package '{}'...", packageId);
    SetupPackage(packagePath, binariesPath, packageId);
    stdext::success("Package '{}' repaired successfully.", packageId);
}
