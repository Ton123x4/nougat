#include "utils.hpp"
#include <miniz.h>
#include <curl/curl.h>
#include <stdext/filesystem.hpp>
#include <stdext/fstream.hpp>
#include <stdext/string.hpp>
#include <stdext/logger.hpp>
#include <filesystem>

namespace fs = std::filesystem;

auto WriteCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
    if (dltotal <= 0) {
        return 0;
    }

    stdext::update("Downloading {}%", (dlnow * 100) / dltotal);
    return 0;
}

void DownloadPackage(const std::string& fileURL, const std::string& outputPath) {
    auto file = stdext::file::open(outputPath, "wb");

    if (!file) {
        throw std::runtime_error(std::format("Failed to open temporary file for download: {}", outputPath));
    }

    auto curl = curl_easy_init();

    if (!curl) {
        stdext::file::close(file);
        throw std::runtime_error("Failed to initialize libcurl");
    }

    #if defined(_WIN32)
    curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
    #endif

    curl_easy_setopt(curl, CURLOPT_URL, fileURL.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);

    auto result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    stdext::file::close(file);

    if (result != CURLE_OK) {
        throw std::runtime_error(std::format("Download failed: {}", curl_easy_strerror(result)));
    }

    stdext::log("Downloading 100%");
}

std::string FindRoot(mz_zip_archive& archive, mz_uint entryCount) {
    auto rootDir = std::string();

    for (mz_uint i = 0; i < entryCount; i++) {
        auto stat = mz_zip_archive_file_stat();

        if (!mz_zip_reader_file_stat(&archive, i, &stat)) {
            continue;
        }

        auto filename = std::string_view(stat.m_filename);
        auto slash = filename.find('/');

        if (slash == std::string_view::npos) {
            return "";
        }

        auto firstDir = std::string(filename.substr(0, slash + 1));

        if (rootDir.empty()) {
            rootDir = firstDir;
        }
        else if (rootDir != firstDir) {
            return "";
        }
    }

    return rootDir;
}

void ExtractPackage(const std::string& filePath, const std::string& outputPath) {
    auto archive = mz_zip_archive();

    mz_zip_zero_struct(&archive);

    stdext::debug("Opening archive: {}", filePath);
    stdext::debug("Output path: {}",  outputPath);

    if (!mz_zip_reader_init_file(&archive, filePath.c_str(), 0)) {
        throw std::runtime_error(std::format("Failed to open zip archive '{}': {}", filePath, mz_zip_get_error_string(mz_zip_get_last_error(&archive))));
    }

    auto entryCount = mz_zip_reader_get_num_files(&archive);
    auto rootPrefix = FindRoot(archive, entryCount);

    stdext::debug("Entries: {}", entryCount);
    stdext::debug("Root prefix: '{}'", rootPrefix.empty() ? "(none)" : rootPrefix);

    for (mz_uint i = 0; i < entryCount; i++) {
        stdext::update("Installing {}%", ((i + 1) * 100) / entryCount);

        auto stat = mz_zip_archive_file_stat();

        if (!mz_zip_reader_file_stat(&archive, i, &stat)) {
            mz_zip_reader_end(&archive);
            throw std::runtime_error(std::format("Failed to stat entry #{}", i));
        }

        auto filename = std::string(stat.m_filename);

        if (!rootPrefix.empty()) {
            if (filename == rootPrefix) {
                continue;
            }

            if (filename.starts_with(rootPrefix)) {
                filename = filename.substr(rootPrefix.size());
            }
        }

        if (filename.empty()) {
            continue;
        }

        auto entryPath = stdext::fs::sanitize(std::format("{}/{}", outputPath, filename));

        if (filename.ends_with('/')) {
            fs::create_directories(entryPath);
            continue;
        }

        fs::create_directories(fs::path(entryPath).parent_path());

        if (!mz_zip_reader_extract_to_file(&archive, i, entryPath.c_str(), 0)) {
            mz_zip_reader_end(&archive);
            throw std::runtime_error(std::format("Failed to extract entry '{}' from archive", filename));
        }
    }

    mz_zip_reader_end(&archive);

    stdext::log("Installing 100%");
}

void CreatePackageLink(std::string targetPath, const std::string& linkPath) {
    targetPath = stdext::fs::canonical(stdext::fs::absolute(targetPath));

    if (!fs::exists(targetPath)) {
        throw std::runtime_error(std::format("Target path does not exist: '{}'", targetPath));
    }

    stdext::debug("Creating symlink: {} => {}", linkPath, targetPath);

    fs::create_directories(stdext::fs::parent_path(linkPath));
    fs::create_directory_symlink(targetPath, linkPath);
}

void RemovePackageDirectory(const std::string& targetPath) {
    if (fs::is_symlink(targetPath)) {
        fs::remove(targetPath);
    } else {
        fs::remove_all(targetPath);
    }
}
