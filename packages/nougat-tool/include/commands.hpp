#pragma once

#include <stdext/arg_parser.hpp>

void HandleInstall(stdext::arg_parser& argParser);
void HandleRepair(stdext::arg_parser& argParser);
void HandleUninstall(stdext::arg_parser& argParser);
void HandleCommands(stdext::arg_parser& argParser);
void HandleList(stdext::arg_parser& argParser);
