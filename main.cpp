#include "io.hpp"
#include "cache_commands.hpp"
#include <exception>

int main(int argc, char *argv[])
{

    if (argc != 6 && argc != 7)
    {
        cout << "Error: Incorrect # of arguments" << std::endl;
        cout << "[sets] [lines/set] [bytes/line] [write/no-write-allocate] [write-through/write-back] [fifo/lru]" << std::endl;
        return 1;
    }
    cache_info info;
    info.set_count = std::atoi(argv[1]);
    info.lines_per_set = std::atoi(argv[2]);
    info.bytes_per_line = std::atoi(argv[3]);
    info.words_per_line = info.bytes_per_line / 4;
    info.write_allocate = argv[4];
    info.write_through = argv[5];
    // Associative Cache, else direct cache so eject type doesn't matter
    info.lines_per_set > 1 ? info.eject_type = argv[6] : info.eject_type = "fifo";
    if (!validate_info(info))
    {
        cout << "Error: Invalid Parameters\n";
        return 1;
    }
    cache_cmds cache_commands;
    readInput(cache_commands);
    shared_ptr<Cache> cache = simulate_cache(cache_commands, info);
    print_output(cache->get_cache_stats());
    return 0;
}