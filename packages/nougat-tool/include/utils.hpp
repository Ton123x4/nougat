#pragma once

#include <string>

void DownloadPackage(const std::string& fileURL, const std::string& outputPath);

void ExtractPackage(const std::string& filePath, const std::string& outputPath);

void CreatePackageLink(std::string targetPath, const std::string& linkPath);

void RemovePackageDirectory(const std::string& targetPath);
