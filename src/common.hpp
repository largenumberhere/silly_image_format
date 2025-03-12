#pragma once

#include <vector>

std::vector<const char*> collectArgs(int argc, char* argv[]);

void exitWithUsage();

void todoImpl(int line, const char* file);
#define TODO() \
    todoImpl(__LINE__, __FILE__)
