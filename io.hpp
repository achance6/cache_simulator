#ifndef _IO
#define _IO

#include "cache.hpp"
#include <iostream>
#include <sstream>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

typedef vector<cache_cmd> cache_cmds;
bool validate_info(cache_info info);
void read_input(cache_cmds &cmds);
void print_output(cache_stats out);

#endif
