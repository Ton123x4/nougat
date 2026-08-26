#include "commands.hpp"
#include "system.hpp"
#include <stdext/database.hpp>
#include <stdext/memory.hpp>
#include <stdext/console.hpp>
#include <stdext/filesystem.hpp>
#include <stdext/logger.hpp>
#include <filesystem>

namespace fs = std::filesystem;

void HandleCommands(stdext::arg_parser& argParser) {
    auto binariesPath = GetNougatBinaries();

    if (!fs::exists(binariesPath)) {
        stdext::log("No commands installed.");
        return;
    }

    auto globalRegistry = stdext::make_safe(OpenNougatRegistry(), CloseDatabase);
    auto globalSymbols = stdext::make_safe(OpenNougatSymbols(), CloseDatabase);
    auto hasCommands = false;

    for (const auto& entry : fs::directory_iterator(binariesPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto entryPath = entry.path();
        auto fileName = entryPath.filename();
        auto filePath = stdext::db::get(globalSymbols.get(), fileName.string());
        auto packageId = stdext::db::get(globalRegistry.get(), fileName.string());

        if (packageId && filePath) {
            auto packageIdentifier = stdext::fg_color(stdext::ansi_color::bright_blue, packageId);
            auto executableName = stdext::fg_color(stdext::ansi_color::bright_yellow, fileName.string());
            auto executablePath = stdext::fg_color(stdext::ansi_color::white, filePath);

            stdext::println(stdext::ansi_color::default_color, "  [{}] {}", packageIdentifier, executableName);
            stdext::println(stdext::ansi_color::default_color, "    {}", executablePath);
        }
        else {
            auto statusMessage = stdext::fg_color(stdext::ansi_color::red, "unregistered");
            auto executableName = stdext::fg_color(stdext::ansi_color::bright_yellow, fileName.string());
            auto executablePath = stdext::fs::join_path(binariesPath, fileName);

            stdext::println(stdext::ansi_color::red, "  [{}] {}", statusMessage, executableName);
            stdext::println(stdext::ansi_color::default_color, "    {}", executablePath);
        }

        hasCommands = true;
    }

    if (!hasCommands) {
        stdext::log("No commands installed.");
    }
}

void HandleList(stdext::arg_parser& argParser) {
    auto packagesPath = GetNougatPackages();

    if (!fs::exists(packagesPath)) {
        stdext::info("No packages installed.");
        return;
    }

    auto hasPackages = false;

    for (const auto& entry : fs::directory_iterator(packagesPath)) {
        if (!entry.is_directory() && !entry.is_symlink()) {
            continue;
        }

        auto entryPath = entry.path();
        auto fileName = entryPath.filename();
        auto isSymlink = fs::is_symlink(entryPath);
        auto packageName = stdext::fg_color(stdext::ansi_color::bright_blue, fileName.string());

        if (isSymlink) {
            auto target = fs::read_symlink(entryPath);
            auto linkIndicator = stdext::fg_color(stdext::ansi_color::white, std::format("-> {}", target.string()));
            stdext::println(stdext::ansi_color::default_color, "  {} {}", packageName, linkIndicator);
        }
        else {
            stdext::println(stdext::ansi_color::default_color, "  {}", packageName);
        }

        hasPackages = true;
    }

    if (!hasPackages) {
        stdext::info("No packages installed.");
    }
}
