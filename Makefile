all: xenon

clean:
	rm -rf build/* bios.* examples.* xge xenon buildxen gmon.out *.bin *.map

xenon: src/*.c bios.bin
	gcc -O3 -g src/*.c -o xenon -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow -pg

build/%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@
	
bios.bin: build/bios.obj
	ld65 -v -o bios.bin build/*.obj -m bios.map -C conf/xenon_bios.ld

install: xenon
	mkdir -p /usr/local/share/xenon
	mkdir -p /usr/local/include/xenon
	
	cp xenon /usr/local/bin/
	cp bios.bin /usr/local/share/xenon
	
	#install bios headers/api
	cp bios/xenon_def.s /usr/local/include/xenon/
	cp bios/ram.s       /usr/local/include/xenon/
	