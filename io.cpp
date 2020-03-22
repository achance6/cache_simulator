#include "io.hpp"
#include "cache.hpp"
#include <istream>

bool validate_info(cache_info info)
{
  if (ceil(log2(info.set_count)) != floor(log2(info.set_count)))
    return false; // sets must be power of 2
  if (ceil(log2(info.lines_per_set)) != floor(log2(info.lines_per_set)))
    return false; // lines_ps must be power of 2
  if (info.bytes_per_line < 4 || (ceil(log2(info.bytes_per_line)) != floor(log2(info.bytes_per_line))))
    return false; // bytes_ps must be power of 2 and at least 4
  if (info.write_allocate.compare("write-allocate") != 0 && info.write_allocate.compare("no-write-allocate") != 0)
    return false; // write_allocate must be write-allocate or no-write-allocate
  if (info.write_through.compare("write-through") != 0 && info.write_through.compare("write-back") != 0)
    return false; // write_through must be write-through or write-back
  if (info.lines_per_set > 1 && (info.eject_type.compare("lru") != 0 && info.eject_type.compare("fifo") != 0))
    return false; // if lines_per_set > 1 then eject type must be given
  if (info.lines_per_set == 1 && info.eject_type.compare("fifo") != 0)
    return false; // if lines_per_set is 1 then eject type must be empty
  return true;
}

void read_input(vector<cache_cmd> &cmds)
{
  char ls;
  string address;
  cache_cmd cmd;

  while (cin >> ls && cin >> address)
  {
    cmd.ls = (char)ls;
    cmd.address = address;
    cin.ignore(10000, '\n'); //skip third command
    cmds.push_back(cmd);
  }
}

void print_output(cache_stats out)
{
  cout << "Total loads: " << out.total_loads << endl;
  cout << "Total stores: " << out.total_stores << endl;
  cout << "Load hits: " << out.load_hits << endl;
  cout << "Load misses: " << out.load_misses << endl;
  cout << "Store hits: " << out.store_hits << endl;
  cout << "Store misses: " << out.store_misses << endl;
  cout << "Total cycles: " << out.total_cycles << endl;
}