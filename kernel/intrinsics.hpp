#pragma once

#include <stl/types.hpp>

inline void outb(u16 port, u8 value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline void halt() {
    asm ("cli");
    while (true) {
        asm ("hlt");
    }
}