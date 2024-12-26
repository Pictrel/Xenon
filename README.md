The XENON is an 8-bit fantasy console inspired by the [Famicom Disk System](https://en.wikipedia.org/wiki/Famicom_Disk_System) powered by a 6502 and 128K diskettes.

## Specs

- CPU: 6502 @ 10 MHz
- RAM: 48K
- Resolution: 200 x 150 @ 2bpp
- Palettes: 4 for BG, 4 for sprites, each color is RGB332

## Building

Building the XENON requires installing [m6502](https://github.com/redcode/6502) and [raylib](https://github.com/raysan5/raylib).

After installing said libraries, run `make` to build `xenon` and `bios.bin`, and `make install` to install them.

## Debugger

The XENON has a built-in debugger, which lets you step through the code, disassemble it, and view the memory.
