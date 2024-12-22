all: xenon xge calendar.xen

clean:
	rm -rf build/* bios.* calendar.* xge xenon buildxen 

xenon: src/*.c bios.bin
	clang -O3 -g src/*.c -o xenon -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow -pg

bios.bin: build/b_bios.obj
	ld65 -v -o bios.bin build/b_*.obj -m bios.map -C conf/xenon_bios.ld

build/b_%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@

build/c_%.obj: calendar/%.s
	ca65 --cpu 6502 --verbose $< -o $@

%: tools/%.c
	clang $< -o $@ -lraylib -lm `pkg-config --cflags --libs libconfig` -g

calendar.bin: build/c_cal.obj
	ld65 -v -o calendar.bin build/c_*.obj -m calendar.map -C conf/xenon_disc.ld

calendar.xen: calendar.bin buildxen
	./buildxen -o calendar.xen conf/calendar.cfg