all: r6502 xge

clean:
	rm -rf build/* bios.* xge r6502

r6502: src/*.c bios.bin
	gcc src/*.c -o r6502 -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow

bios.bin: build/bios.obj
	ld65 -v -o bios.bin build/*.obj -m bios.map -C conf/xenon_bios.ld

build/%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@

%: tools/%.c
	gcc $< -o $@ -lraylib -lm