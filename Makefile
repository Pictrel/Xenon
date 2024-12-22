all: xenon xge buildxen calendar.xen

clean:
	rm -rf build/* bios.* xge xenon buildxen

xenon: src/*.c bios.bin
	clang -O3 -g src/*.c -o xenon -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow -pg

bios.bin: build/bios.obj
	ld65 -v -o bios.bin build/*.obj -m bios.map -C conf/xenon_bios.ld

build/%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@

build/cal_%.obj: calendar/%.s
	ca65 --cpu 6502 --verbose $< -o $@

%: tools/%.c
	clang $< -o $@ -lraylib -lm `pkg-config --cflags --libs libconfig` -g

calendar.xen: build/
	