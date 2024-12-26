#include <stdint.h>
#include <6502.h>

#pragma once

extern uint8_t ram[0xC000];
extern uint8_t vram[0x2000];
extern uint8_t bios[0x1000];
extern uint8_t bus;

extern uint8_t t_clock;

extern uint8_t IO_TCNT;
extern uint8_t IO_TCTL;
extern uint8_t IO_TDIV;
extern uint8_t IO_VCTL;
extern uint8_t IO_VSTA;
extern uint8_t IO_VMX;
extern uint8_t IO_VMY;
extern uint8_t IO_VY;
extern uint8_t IO_VYC;
extern uint8_t IO_VWL;
extern uint8_t IO_VWR;

extern uint8_t IO_FCTL;


/*

Oeeeemmm

O: motor on
e: error code
	0 = OK
	1 = invalid command
	2 = read error
	3 = write error
	4 = no disk
	5 = busy
	8 = OS error
	
	E = motor not on
	F = undefined error
m: mode
	0 = off
	1 = idle
	4 = initializing
	5 = seeking
	6 = reading
	7 = writing
*/

extern uint8_t IO_FDAT;
extern uint8_t IO_FCMD;
extern uint8_t IO_ICTL;
/*
An NMI is fired every VBLANK to give time for the CPU to update data on screen.
As for the IRQ line, the CPU can select the desired interrupt source.

y-----ft

y: Select VYC interrupt 
t: Select Timer interrupt (when TCNT overflows)
f: Select Floppy interrupt
*/

//uint8_t IO_ISTA;
/*
When an interrupt is fired, this register is set depending on the source.

-----sss

S:
	0 = timer interrupt
	1 = floppy interrupt
	7 = vyc interrupt
*/

extern int irq_source;

extern int fps;

uint8_t joy1();
uint8_t joy2();
uint8_t get_FSTA();
uint8_t get_ISTA();

void cpu_iowrite(uint8_t addr, uint8_t val);
uint8_t cpu_ioread(uint8_t addr);
zuint8 cpu_read(void *ctx, zuint16 addr);
void cpu_write(void *ctx, zuint16 addr, zuint8 val);
