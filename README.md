# mat-os

its mat-os again. this time using limine on x86_64 because i don't feel like making a bootloader, and limine does most of the boring work

![image](https://github.com/matcool/mat-os/assets/26722564/1debab95-37ce-4dac-8c10-95d87313c0ff)
*screenshot of the system as of [`c9585bb`](https://github.com/matcool/mat-os/commit/c9585bb5f9bc36158e466f83e127f7db2c9d58ec)*

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
- [X] Kernel heap allocator
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
