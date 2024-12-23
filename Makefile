all: xenon xge examples.xen

clean:
	rm -rf build/* bios.* examples.* xge xenon buildxen gmon.out *.bin *.map

xenon: src/*.c bios.bin
	clang -O3 -g src/*.c -o xenon -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow -pg

bios.bin: build/b_bios.obj
	ld65 -v -o bios.bin build/b_*.obj -m bios.map -C conf/xenon_bios.ld

build/b_%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@

build/e_%.obj: examples/%.s
	ca65 --cpu 6502 --verbose $< -o $@

%: tools/%.c
	clang $< -o $@ -lraylib -lm `pkg-config --cflags --libs libconfig` -g

hello.bin: build/e_hello.obj
	ld65 -v -o hello.bin build/e_hello.obj -m hello.map -C conf/xenon_disc.ld

examples.xen: hello.bin buildxen
	./buildxen -o examples.xen conf/examples.cfg