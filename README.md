# mat-os

its mat-os again. this time using limine on x86_64 because it does most of the boring work

![image](https://github.com/matcool/mat-os/assets/26722564/ff69e7d4-beb1-4ad0-832d-48561a57d471)
*screenshot of the system as of [`63a99e9`](https://github.com/matcool/mat-os/commit/63a99e93feac7b13c74c06b96adad929943f5622)*

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
- [X] PS/2 mouse input
- [X] GUI system
- [ ] A basic in-memory filesystem

### Apps
- [ ] Use ELF for apps?
- [ ] A simple text editor
- [ ] File explorer app

### Misc
- [ ] QOI image support
