#pragma once

#include <vector>

std::vector<const char*> collectArgs(int argc, char* argv[]);
void exitWithUsage();