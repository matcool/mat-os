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
- [ ] PS/2 keyboard input
- [ ] Working timer (to time the screen)
- [X] Working screen
- [ ] Basic terminal interface on screen
- [ ] PS/2 mouse input
- [ ] Drawing to the screen
- [ ] Begin windowing system
- [ ] A basic in-memory filesystem
- [ ] GUI system

### Apps
- [ ] Use ELF for apps?
- [ ] A simple text editor
- [ ] File explorer app

### Misc
- [ ] QOI image support