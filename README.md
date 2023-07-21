# mat-os

its mat-os again. this time using limine on x86_64 because i don't feel like making a bootloader, and limine does most of the boring work

## Goals
- [ ] ~~Use C++20 modules~~ doesn't work with clangd :(
- [X] Use CMake
- [X] Serial output
- [X] Working IDT
- [X] Physical page allocator (very inefficient)
- - [ ] A better Physical page allocator
- [X] Paging (though basic)
- [X] Virtual page allocator (bump allocator, can't free)
- - [ ] A better virtual page allocator
- [X] PS/2 keyboard input
- - [ ] Some way to get key events out of the interrupt
- [X] Working timer (PIT)
- - [ ] Events? Scheduling?
- [X] Working screen
- [X] Basic on screen "terminal"
- - [ ] Proper terminal interface with commands and such
- [X] Drawing to the screen
- [ ] Kernel heap allocator
- [ ] Begin windowing system
- [ ] PS/2 mouse input
- [ ] GUI system
- [ ] A basic in-memory filesystem

### Apps
- [ ] Use ELF for apps?
- [ ] A simple text editor
- [ ] File explorer app

### Misc
- [ ] QOI image support