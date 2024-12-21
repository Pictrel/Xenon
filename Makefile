all: xenon xge

clean:
	rm -rf build/* bios.* xge xenon

xenon: src/*.c bios.bin
	clang src/*.c -o xenon -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow

bios.bin: build/bios.obj
	ld65 -v -o bios.bin build/*.obj -m bios.map -C conf/xenon_bios.ld

build/%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@

%: tools/%.c
	gcc $< -o $@ -lraylib -lm