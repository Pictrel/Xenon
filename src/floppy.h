#include <stdint.h>
#include <6502.h>

#pragma once

bool fdd_cycle();

extern bool fd_motor;
extern int fd_err;
extern int fd_mode;
extern int fd_timer; //used internally to simulate FDD delay
extern int fd_pos;
extern int fd_arg;
extern int fd_idx;