#ifndef _CACHE_COMMANDS
#define _CACHE_COMMANDS
#include "cache.hpp"
#include <memory>

using std::shared_ptr;
shared_ptr<Cache> simulate_cache(vector<cache_cmd> cmds, cache_info info);

#endif