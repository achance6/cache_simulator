#ifndef _CACHE
#define _CACHE

#include "cache_defs.hpp"
#include <memory>

using std::shared_ptr;

class Cache
{
public:
    Cache(cache_info info);
    bool load(uint32_t address);
    bool store(uint32_t address);
    uint32_t get_cache_type();
    cache_stats get_cache_stats() { return stats; }

private:
    uint32_t index_bit_count;
    uint32_t offset_bit_count;
    uint32_t tag_bit_count;
    cache_info info;
    map<INDEX, CACHE_SET> sets;
    cache_stats stats;
    uint32_t get_tag(uint32_t address){ return address >> (index_bit_count + offset_bit_count); }
    uint32_t get_index(uint32_t address) { return (address >> offset_bit_count) & ((1 << index_bit_count) - 1); }
    uint32_t get_offset(uint32_t address) { return address & ((1 << offset_bit_count) - 1); }
    void distribute_bits();
    void init_sets();
    void FIFO_miss(uint32_t address, CACHE_SET &set);
    void LRU_miss(uint32_t address, CACHE_SET &set);
    void LRU_shift(cache_line& line, CACHE_SET &set);
    void LRU_increment(CACHE_SET &set);
    void print_sets(uint32_t address, CACHE_SET &set);
};


shared_ptr<Cache> simulate_cache(vector<cache_cmd> cmds, cache_info info);

#endif