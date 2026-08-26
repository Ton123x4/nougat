#pragma once

#include <stdext_lib.h>
#include <stdext/database.hpp>
#include <string>
#include <vector>


STDEXT_LIB_API void InitNougatSystem();


STDEXT_LIB_API const std::string& GetNougatBinaries();

STDEXT_LIB_API const std::string& GetNougatPackages();

STDEXT_LIB_API const std::string& GetNougatBinary();


STDEXT_LIB_API std::string GetBinaryName(const std::string& filename);

STDEXT_LIB_API std::string GetPackageID(const std::string& filename);

STDEXT_LIB_API std::string GetPackagePath(const std::string& packageId);

STDEXT_LIB_API std::string GetEnvironmentPath(const std::string& packageId);

STDEXT_LIB_API std::string GetExecutablePath(const std::string& filename);


STDEXT_LIB_API stdext::database* OpenPackageEnvironment(const std::string& identifier);

STDEXT_LIB_API stdext::database* OpenNougatRegistry();

STDEXT_LIB_API stdext::database* OpenNougatSymbols();

STDEXT_LIB_API void CloseDatabase(stdext::database* db);


STDEXT_LIB_API int ExecuteApplication(const std::string& identifier, const std::vector<std::string>& arguments);
