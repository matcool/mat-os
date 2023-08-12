# mat-os

its mat-os again. this time using limine on x86_64 because it does most of the boring work

![image](https://github.com/matcool/mat-os/assets/26722564/60d84f2d-00ac-402a-afeb-0cf3ee1d8181)
*screenshot of the system as of [`b8c5541`](https://github.com/matcool/mat-os/commit/b8c5541e895e965b56acf68b913e72c4bd11475e)*

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
- [X] Begin windowing system
- - [X] Fix that paint trail bug
- - [ ] Proper GUI system
- [X] PS/2 mouse input
- [ ] Threads
- [ ] Processes
- [ ] A basic in-memory filesystem
- [ ] Begin user-space

### Apps
- [ ] Use ELF for apps?
- [ ] A simple text editor
- [ ] File explorer app

### Misc
- [ ] QOI image support
