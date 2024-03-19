#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdbool.h>
#include <stdint.h>

#define SB_ADDR 0xFF01
#define SC_ADDR  0xFF02

extern uint8_t SB_REG;
extern uint8_t SC_REG;

void initSerial();
void updateSerial();
void freeSerial();
void updateSerial();
bool load_network_config();

#endif