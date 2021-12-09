#include "stdio.hpp"
#include "terminal.hpp"

void printf(const char* fmt, ...) {
    terminal_write_string(fmt);
}