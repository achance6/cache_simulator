#include "cache.hpp"
#include "io.hpp"

shared_ptr<Cache> simulate_cache(cache_cmds cmds, cache_info info)
{
    auto cache = std::make_shared<Cache>(info);
    for (auto cmd : cmds)
    {
        uint32_t address = (uint32_t)std::stoul(cmd.address, nullptr, 16);
        cmd.ls == 'l' ? cache->load(address) : cache->store(address);;
        // std::cout << "Total Cycles: " << cache->get_cache_stats().total_cycles << std::endl << std::endl;
    }
    return cache;
}

Cache::Cache(cache_info info) : info(info)
{
    distribute_bits();
    stats = {}; // Initialize all stats to 0;
    init_sets();
}

// void Cache::print_sets(uint32_t address, CACHE_SET &set) {
//     // std::cout << "Printing set with index: " << get_index(address) << std::endl;
//     // for (auto line : set)
//     //     {
//     //         std::cout << "tag: " << line.first << " access order: " << line.second.access_order << " | ";
//     //     }   
//         // std::cout << std::endl;
//         // std::cout << "Attempting to load memory..." << std::endl;
//         // std::cout << "tag: " << get_tag(address) << std::endl;
//         // std::cout << "index: " << get_index(address) << std::endl;
//         // std::cout << "offset: " << get_offset(address) << std::endl;
//     }
// Gets the type of cache
uint32_t Cache::get_cache_type()
{
    if (info.lines_per_set == 1)
    {
        return DIRECT_MAPPED;
    }
    else if (info.set_count > 1 && info.lines_per_set > 1)
    {
        return SET_ASSOCIATIVE;
    }
    else
    {
        return FULLY_ASSOCIATIVE;
    }
}

// Executes load command
bool Cache::load(uint32_t address)
{
    stats.total_loads += 1;

    CACHE_SET *set = &sets.at(get_index(address)); // Get set

    // print_sets(address, *set);
    // Look for tag in set
    auto line_containing_word = set->find(get_tag(address));

    // load miss, load line into cache from disk. 
    // I use placeholders tag but sometimes its is actually the memory address, so I must check for that thru valid checks
    if (line_containing_word == set->end() || line_containing_word->second.valid == false)
    {
        stats.load_misses += 1;

        // std::cout << std::endl << "Load miss!" << std::endl;
        // std::cout << "Retrieved from disk!" << std::endl;
        // std::cout << "Fetched from cache!" << std::endl << std::endl;

        stats.total_cycles += (info.words_per_line * 100) + 1; // Fetch line from disk

        if (info.eject_type.compare("") == 0 || info.eject_type.compare("fifo") == 0)
        {
            FIFO_miss(address, *set);
        }
        else
        {
            LRU_increment(*set);
            LRU_miss(address, *set);
        }
    }
    // load hit
    else
    {
        stats.load_hits += 1;
        stats.total_cycles += 1; // read line
        // std::cout << std::endl << "load hit!";
        // std::cout << std::endl << "fetched from cache!" << std::endl << std::endl;
        // LRU must reset the access order of hit memory
        if (info.eject_type.compare("lru") == 0)
        {
            LRU_shift(line_containing_word->second, *set);
            
        }
        return true;
    }
    return true;
}

// Execute store command
bool Cache::store(uint32_t address)
{
    stats.total_stores += 1;
    CACHE_SET *set = &sets.at(get_index(address));

    // DEBUGGING
    // std::cout << "Printing set with index: " << get_index(address) << std::endl;
    // for (auto line : *set)
    // {
    //     std::cout << "tag: " << line.first << " valid: " << line.second.valid << " | ";
    // }   
    // std::cout << std::endl;
    // std::cout << "Attempting to store memory..." << std::endl;
    // std::cout << "tag: " << get_tag(address) << std::endl;
    // std::cout << "index: " << get_index(address) << std::endl;
    // std::cout << "offset: " << get_offset(address) << std::endl << std::endl;
    // END DEBUGGING

    bool is_write_allocate = info.write_allocate.compare("write-allocate") == 0;
    bool is_write_through = info.write_through.compare("write-through") == 0;
    bool is_lru = info.eject_type.compare("lru") == 0;
    auto mem_line = set->find(get_tag(address));
    // cache contains tag, so memory contains tag
    if (mem_line != set->end() && mem_line->second.valid) {
        stats.store_hits += 1; 
        // std::cout << "store hit!" << std::endl << std::endl;
        if (is_write_through) {
            stats.total_cycles += 100; // write word to cache and disk in parallel
        }
        // write-back
        else {
            stats.total_cycles += 1; //update line to be dirty
            mem_line->second.dirty = true;
        }

        if (is_lru) LRU_shift((set->find(get_tag(address))->second), *set);
        mem_line->second.access_order = 0;

        return true;
    }
    // store miss
    else 
    {
        stats.store_misses += 1; 
        // std::cout << "store miss!" << std::endl;

        if (is_write_allocate) {
            stats.total_cycles += (100 * info.words_per_line); //parallel load from disk and load to cache
            if (is_lru) {
                LRU_increment(*set);
                LRU_miss(address, *set);
            }
            else {
                FIFO_miss(address, *set); 
            }
            // std::cout << "Load to cache from disk" << std::endl; // We must fetch memory block from disk
            if (is_write_through) {
                stats.total_cycles += 100; // parallel write to cache and memory
                // std::cout << "Wrote to cache and disk" << std::endl << std::endl;
            }
            else if (!is_write_through) {
                stats.total_cycles += 1; //write to cache only, mark as dirty
                // std::cout << "Wrote to cache, set line to dirty" << std::endl << std::endl;
                set->find(get_tag(address))->second.dirty = true;
            }
        }
        //no-write-allocate
        else {
            stats.total_cycles += 100; //write to disk only, cache unchanged
        }
    }
    return true;
}

// Fill map of sets out with sets associated with an index, and fill those sets with empty line(s).
void Cache::init_sets()
{
    auto set_count = info.set_count;
    auto lines_per_set = info.lines_per_set;
    for (uint32_t i = 0; i < set_count; ++i)
    {
        CACHE_SET set;
        for (uint32_t j = 0; j < lines_per_set; ++j)
        {
            set.insert(pair<INDEX, cache_line>(j, cache_line())); // insert empty lines to fill out set
        }
        sets.insert(pair<INDEX, CACHE_SET>(i, set)); // insert set into cache sets
    }
}

// Computes bits needed for tag, index, and offset
void Cache::distribute_bits()
{
    switch (get_cache_type())
    {
    case DIRECT_MAPPED:
    case SET_ASSOCIATIVE:
        index_bit_count = log2(info.set_count);
        offset_bit_count = log2(info.bytes_per_line);
        tag_bit_count = DISK_BITS - index_bit_count - offset_bit_count;
        break;
    case FULLY_ASSOCIATIVE:
        index_bit_count = 0;
        offset_bit_count = log2(info.bytes_per_line);
        tag_bit_count = DISK_BITS - offset_bit_count;
        break;
    }
}

// Insertion method of First In First Out
void Cache::FIFO_miss(uint32_t address, CACHE_SET &set)
{
    uint32_t oldest_order = 0;
    TAG oldest_tag;
    TAG hole_tag;
    bool hole_exists = false;
    // Increment all valid lines' queue orders
    for (auto& line : set)
    {
        line.second.insertion_order += 1;
        // Keep track of who was the first (valid) line to enter queue.
        if (line.second.insertion_order > oldest_order && line.second.valid)
        {
            oldest_tag = line.first;
            oldest_order = line.second.insertion_order;
        }
        if (!line.second.valid)
        {
            hole_tag = line.first;
            hole_exists = true;
        }
    }
    // Insert/overwrite line
    // The only case an overwrite occurs is if a tag is the same as a placeholder tag.
    // Queue order is 0 for inserted line.
    set[get_tag(address)] = cache_line(get_offset(address));
    

    // Size has exceeded allowed lines, must remove either a hole if it exists or the oldest tag
    if (set.size() > info.lines_per_set)
    {
        // hole_exists ? std::cout << "Erased tag hole: " << hole_tag << std::endl : std::cout << "Erased tag: " << oldest_tag << std::endl;
        if (hole_exists)
        {
            set.erase(hole_tag);
        }
        else
        {
            if (set[oldest_tag].dirty) {
                stats.total_cycles += (100 * info.words_per_line); // dirty word indicates that we need to write entire line to memory before erasing
                // std::cout << "Evicted dirty line, wrote to disk" << std::endl;
            }
            set.erase(oldest_tag);
        }
    }
}

// Least Recently Used insertion method. If a line needs to be then inserted line has access order of zero.
// If a line already exists 
// If there is a hole we don't need to evict, if there is not then we evict one with highest access order (back of the line).
void Cache::LRU_miss(uint32_t address, CACHE_SET &set)
{
    uint32_t oldest_access_order = 0;
    TAG least_recently_used_tag;
    TAG hole_tag;
    bool hole_exists = false;
    for (auto& line : set)
    {
        // Keep track of who was the first (valid) line to enter queue.
        if ((line.second.access_order >= oldest_access_order) && line.second.valid)
        {
            least_recently_used_tag = line.first;
            oldest_access_order = line.second.access_order;
        }
        if (!line.second.valid)
        {
            hole_tag = line.first;
            hole_exists = true;
        }
    }

    // if line already exists, do nothing. resetting of access order is handled in load function
    // If i reset access order here than a store would erroneously reset the access order
    if (set.find(get_tag(address)) == set.end() || !set[get_tag(address)].valid) {
        set[get_tag(address)] = cache_line(get_offset(address));
    }
    if (set.size() > info.lines_per_set)
    {
        // hole_exists ? std::cout << "Erased tag hole: " << hole_tag << std::endl : std::cout << "Erased tag: " << least_recently_used_tag << std::endl;
        if (hole_exists)
        {
            set.erase(hole_tag);
        }
        else
        {
            if (set[least_recently_used_tag].dirty) {
                stats.total_cycles += 100 * info.words_per_line; // dirty line indicates that we need to write to memory before erasing
                // std::cout << "Evicted dirty block, wrote to disk" << std::endl;
            }
            set.erase(least_recently_used_tag);
        }
    }
}

// Brings the given line to the front and increments the orders of all lines in front of it.
void Cache::LRU_shift(cache_line& line, CACHE_SET &set)
{
    uint32_t line_access_order = line.access_order;
    for (auto& other_line : set)
    {
        if (other_line.second.valid && (other_line.second.access_order < line_access_order))
        {
            other_line.second.access_order += 1;
        }
    }
    line.access_order = 0;
}

// Increments the access order of every line in the set
void Cache::LRU_increment(CACHE_SET &set) {
    for (auto& line: set) {
        line.second.access_order += 1;
    }
}

