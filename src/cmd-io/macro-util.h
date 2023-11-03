#pragma once

#include "system/angband.h"

#include <string>
#include <vector>

extern std::vector<char> macro_buffers;

extern bool get_com_no_macros;

extern std::vector<std::string> macro_patterns;
extern std::vector<std::string> macro_actions;
extern int16_t active_macros;

int macro_find_exact(concptr pat);
int macro_find_check(concptr pat);
errr macro_add(concptr pat, concptr act);
int macro_find_maybe(concptr pat);
int macro_find_ready(concptr pat);
