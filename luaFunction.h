#pragma once

#include <vector>
#include <string>

std::vector<std::string > LuaParseMKVContent(const char * name, char * buffer, uint64_t size);

#include "MKVValue.h"

MKVValue *LuaGetMKVValue_(VINT id);