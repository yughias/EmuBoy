#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdbool.h>
#include <stdint.h>

#define SB_ADDR 0xFF01
#define SC_ADDR 0xFF02

typedef struct gb_t gb_t;
typedef enum SERIAL_MODE { SLAVE = 0, MASTER } SERIAL_MODE;

typedef struct serial_t {
    uint8_t SB_REG;
    uint8_t SC_REG;
    size_t counter;
    SERIAL_MODE mode;
} serial_t;

void initSerial();
void updateSerial(gb_t*);
void freeSerial();
void updateSerial(gb_t*);
bool load_network_config();

#endif