all: xenon

# replace this with directory where krypton is actually installed
KRYPTON := ../Krypton

clean:
	rm -rf build/* bios.* examples.* xge xenon* buildxen gmon.out *.bin *.map

xenon: src/*.c bios.bin bios.lab xenon.lab
	gcc -O3 -g src/*.c -o xenon -l6502 -lraylib -lm -pedantic -Wall -Wno-overflow -pg -Wno-implicit-function-declaration
	
xenon-rel: src/*.c bios.bin bios.lab xenon.lab
	gcc -O3 src/*.c -o xenon-rel -pedantic -Wall -Wno-overflow /usr/lib/lib6502.a \
	                                                        /usr/local/lib/libraylib.a \
	                                                        /usr/lib/x86_64-linux-gnu/libdl.a -lm
bios.lab: bios.lst
	$(KRYPTON)/ParseLabels bios.lst | tee bios.lab

xenon.lab:
	cp conf/xenon.lab .

build/%.obj: bios/%.s
	ca65 --cpu 6502 --verbose $< -o $@ -l bios.lst
	
bios.bin: build/bios.obj
	ld65 -v -o bios.bin build/*.obj -m bios.map -C conf/xenon_bios.ld

install: xenon
	mkdir -p /usr/local/share/xenon
	mkdir -p /usr/local/include/xenon
	
	cp xenon     /usr/local/bin/
	cp bios.bin  /usr/local/share/xenon
	cp bios.lab  /usr/local/share/xenon
	cp xenon.lab /usr/local/share/xenon
	
	#install bios headers/api
	cp bios/xenon_def.s /usr/local/include/xenon/
	cp bios/ram.s       /usr/local/include/xenon/
	