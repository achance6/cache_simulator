#ifndef _CACHE_DEFS
#define _CACHE_DEFS

#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <iostream>

using std::map;
using std::pair;
using std::string;
using std::vector;

const uint32_t DISK_BITS = 32;
const uint32_t DIRECT_MAPPED = 0;
const uint32_t SET_ASSOCIATIVE = 1;
const uint32_t FULLY_ASSOCIATIVE = 2;
const uint32_t CACHE_CYCLES = 1;
const uint32_t DISK_CYCLES = 100;

struct cache_cmd
{
    char ls;
    string address;
};
struct cache_stats
{
    uint32_t total_loads;
    uint32_t total_stores;
    uint32_t load_hits;
    uint32_t load_misses;
    uint32_t store_hits;
    uint32_t store_misses;
    uint32_t total_cycles;
};
struct cache_info
{
    uint32_t set_count;
    uint32_t lines_per_set;
    uint32_t bytes_per_line;
    uint32_t words_per_line;
    string write_allocate;
    string write_through;
    string eject_type;
};
struct cache_line
{
    cache_line(uint32_t offset) : offset(offset), access_order(0), insertion_order(0), valid(true),  dirty(false){};
    cache_line() : offset(0), access_order(0), insertion_order(0), valid(false),  dirty(false){};
    uint32_t offset;
    uint32_t access_order;
    uint32_t insertion_order;
    bool valid;
    bool dirty;
};
typedef uint32_t TAG;
typedef uint32_t INDEX;
typedef map<TAG, cache_line> CACHE_SET;

#endif