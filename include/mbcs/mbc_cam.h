#ifndef __MBC_CAM_H__
#define __MBC_CAM_H__

#include <stdint.h>

void mbc_cam_init();
void mbc_cam_free();
uint8_t mbc_cam_ram_read(uint16_t addr);
void mbc_cam_ram_write(uint16_t addr, uint8_t byte);

#endif