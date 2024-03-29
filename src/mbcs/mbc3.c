#include "mbc.h"
#include "mbcs/mbc3.h"
#include "memory.h"

#include <time.h>
#include <stdio.h>

uint8_t RTC_08;
uint8_t RTC_09;
uint8_t RTC_0A;
uint8_t RTC_0B;
uint8_t RTC_0C;
uint8_t RTC_08_LATCHED;
uint8_t RTC_09_LATCHED;
uint8_t RTC_0A_LATCHED;
uint8_t RTC_0B_LATCHED;
uint8_t RTC_0C_LATCHED;
uint64_t rtc_timestamp;

void emulateRtc();

#define READ_RTC(addr) case 0x ## addr: return RTC_ ## addr ## _LATCHED
#define WRITE_RTC(addr) case 0x ## addr: RTC_ ## addr = byte; return

uint8_t mbc3_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = MBC_2000_3FFF == 0x00 ? 0x01 : (MBC_2000_3FFF & 0b1111111); 
    real_addr |= bank << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t mbc3_ram_read(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    size_t real_addr = addr;
    switch(MBC_4000_5FFF){
        READ_RTC(08);
        READ_RTC(09);
        READ_RTC(0A);
        READ_RTC(0B);
        READ_RTC(0C);

        default:
        real_addr &= (1 << 13) - 1;
        real_addr |= (MBC_4000_5FFF & 0b11) << 13;
        real_addr &= ERAM_SIZE - 1;
        return ERAM[real_addr];
    }
}

void mbc3_ram_write(uint16_t addr, uint8_t byte){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return;

    size_t real_addr = addr;
    switch(MBC_4000_5FFF){
        WRITE_RTC(08);
        WRITE_RTC(09);
        WRITE_RTC(0A);
        WRITE_RTC(0B);
        
        case 0x0C:    
        if(RTC_0C & (1 << 6))
            rtc_timestamp = time(NULL);
        RTC_0C = byte;
        break;

        default:
        real_addr &= (1 << 13) - 1;
        real_addr |= (MBC_4000_5FFF & 0b11) << 13;
        real_addr &= ERAM_SIZE - 1;
        ERAM[real_addr] = byte;
    }
}

void mbc3_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x2000)
        MBC_0000_1FFF = byte;
    else if(addr < 0x4000)
        MBC_2000_3FFF = byte;
    else if(addr < 0x6000)
        MBC_4000_5FFF = byte;
    else if(hasRtc) {
        if(MBC_6000_7FFF == 0x00 && byte == 0x01){
            emulateRtc();
            RTC_08_LATCHED = RTC_08 & 0b111111;
            RTC_09_LATCHED = RTC_09 & 0b111111;
            RTC_0A_LATCHED = RTC_0A & 0b11111;
            RTC_0B_LATCHED = RTC_0B;
            RTC_0C_LATCHED = RTC_0C & 0b11000001;
        }
        MBC_6000_7FFF = byte;
    }
}

void emulateRtc(){
    if(RTC_0C & (1 << 6))
        return;

    uint64_t elapsed_sec = time(NULL) - rtc_timestamp;
    stepRtc(elapsed_sec);

    if(elapsed_sec != 0)
        rtc_timestamp = time(NULL);
}

void stepRtc(uint64_t secs){
    uint64_t day_counter = RTC_0B + ((RTC_0C & 1) << 8);
    uint64_t rtc_time;
    rtc_time += (RTC_08 % 60);
    rtc_time += ((RTC_09 % 60) * 60);
    rtc_time += ((RTC_0A % 24) * 60 * 60);
    rtc_time += day_counter * 24 * 60 * 60;
    uint64_t new_rtc_time = rtc_time + secs;

    RTC_08 = new_rtc_time % 60;
    RTC_09 = (new_rtc_time / 60) % 60;
    RTC_0A = (new_rtc_time / 3600) % 24;
    uint64_t new_day_counter = new_rtc_time / (24*60*60); 
    bool day_carry = new_day_counter >= 512 ? 1 : 0;
    RTC_0B = new_day_counter % 512;
    RTC_0C &= 0b11000000;
    RTC_0C |= (bool)(new_day_counter & 0x100);
    RTC_0C |= day_carry << 7;
}

// use BGB 64 bit compatible save format 

void saveRtc(const char* filename){
    FILE* fptr = fopen(filename, "ab");

    if(!fptr)
        return;

    uint32_t tmp_RTC_08 = RTC_08; 
    uint32_t tmp_RTC_09 = RTC_09; 
    uint32_t tmp_RTC_0A = RTC_0A; 
    uint32_t tmp_RTC_0B = RTC_0B; 
    uint32_t tmp_RTC_0C = RTC_0C; 
    uint32_t tmp_RTC_08_LATCHED = RTC_08_LATCHED; 
    uint32_t tmp_RTC_09_LATCHED = RTC_09_LATCHED; 
    uint32_t tmp_RTC_0A_LATCHED = RTC_0A_LATCHED; 
    uint32_t tmp_RTC_0B_LATCHED = RTC_0B_LATCHED; 
    uint32_t tmp_RTC_0C_LATCHED = RTC_0C_LATCHED; 

    stepRtc(time(NULL) - rtc_timestamp);
    rtc_timestamp = time(NULL);

    fwrite(&tmp_RTC_08, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_09, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_0A, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_0B, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_0C, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_08_LATCHED, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_09_LATCHED, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_0A_LATCHED, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_0B_LATCHED, sizeof(uint32_t), 1, fptr);
    fwrite(&tmp_RTC_0C_LATCHED, sizeof(uint32_t), 1, fptr);
    fwrite(&rtc_timestamp, sizeof(rtc_timestamp), 1, fptr);
    fclose(fptr);
}

void loadRtc(const char* filename){
    FILE* fptr = fopen(filename, "rb");

    if(!fptr)
        return;

    fseek(fptr, 0, SEEK_END);
    size_t savSize = ftell(fptr);

    if(savSize % 8192 != 48)
        return;

    fseek(fptr, savSize - 48, SEEK_SET);

    uint32_t tmp_RTC_08; 
    uint32_t tmp_RTC_09; 
    uint32_t tmp_RTC_0A; 
    uint32_t tmp_RTC_0B; 
    uint32_t tmp_RTC_0C; 
    uint32_t tmp_RTC_08_LATCHED; 
    uint32_t tmp_RTC_09_LATCHED; 
    uint32_t tmp_RTC_0A_LATCHED; 
    uint32_t tmp_RTC_0B_LATCHED; 
    uint32_t tmp_RTC_0C_LATCHED; 

    fread(&tmp_RTC_08, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_09, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_0A, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_0B, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_0C, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_08_LATCHED, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_09_LATCHED, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_0A_LATCHED, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_0B_LATCHED, sizeof(uint32_t), 1, fptr);
    fread(&tmp_RTC_0C_LATCHED, sizeof(uint32_t), 1, fptr);
    fread(&rtc_timestamp, sizeof(rtc_timestamp), 1, fptr);

    RTC_08 = tmp_RTC_08; 
    RTC_09 = tmp_RTC_09; 
    RTC_0A = tmp_RTC_0A; 
    RTC_0B = tmp_RTC_0B; 
    RTC_0C = tmp_RTC_0C; 
    RTC_08_LATCHED = tmp_RTC_08_LATCHED; 
    RTC_09_LATCHED = tmp_RTC_09_LATCHED; 
    RTC_0A_LATCHED = tmp_RTC_0A_LATCHED; 
    RTC_0B_LATCHED = tmp_RTC_0B_LATCHED; 
    RTC_0C_LATCHED = tmp_RTC_0C_LATCHED; 

    stepRtc(time(NULL) - rtc_timestamp);
    rtc_timestamp = time(NULL);

    fclose(fptr);
}