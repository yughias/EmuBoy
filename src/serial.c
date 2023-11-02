#include <SDL_MAINLOOP.h>
#include "serial.h"
#include "hardware.h"
#include "p2p.h"
#include "ini.h"

#include <stdint.h>
#include <string.h>

P2P_connection p2p;

size_t serial_counter = 0;
enum SERIAL_MODE { SLAVE, MASTER }  serial_mode;

void initSerial(){
    P2P_init(&p2p);
    if(!load_network_config()){
        return;
    }

    SDLNet_Init();

    P2P_establishConnection(&p2p);

    serial_mode = SLAVE;
}

void freeSerial(){
    P2P_close(&p2p);
}

void updateSerial(){
    if(serial_counter--)
        return;

    serial_counter = 4096;

    if(p2p.connection_established){
        switch(serial_mode){
            case SLAVE:
            uint8_t out_byte = SB_REG;
            if((SC_REG & 0x80) && !(SC_REG & 0x01) && P2P_no_blocking_recv(&p2p, &SB_REG, 1)){
                SC_REG &= 0x7F;
                cpu.IF |= SERIAL_IRQ;
                P2P_send(&p2p, &out_byte, 1);  
                return;
            }

            if((SC_REG & 0x80) && (SC_REG & 0x01)){
                P2P_send(&p2p, &SB_REG, 1);
                serial_mode = MASTER;
                return;
            }
            break;

            case MASTER:
            P2P_blocking_recv(&p2p, &SB_REG, 1);
            SC_REG &= 0x7F;
            cpu.IF |= SERIAL_IRQ;
            serial_mode = SLAVE;  
            break;
        }
    } else {
        if((SC_REG & 0x80) && (SC_REG & 0x01)){
            SB_REG = 0xFF;
            SC_REG &= 0x7F;
            cpu.IF |= SERIAL_IRQ;
        }  
    }      
}

bool load_network_config(){
    char ini_path[FILENAME_MAX];
    getAbsoluteDir(ini_path);
    strcat(ini_path, "data/config.ini");

    FILE* ini_ptr = INI_open(ini_path);
    int input_val;

    if(!ini_ptr)
        return false;

    if(INI_getInt(ini_ptr, "listen_port=", &input_val))
        p2p.my_server_port = input_val;
    else
        return false;

    if(INI_getInt(ini_ptr, "connect_port=", &input_val))
        p2p.peer_server_port = input_val;
    else
        return false;

    if(!INI_getString(ini_ptr, "connect_ip=", p2p.peer_server_ip))
        return false;

    return true;
}