#include "SDL_MAINLOOP.h"
#include "serial.h"
#include "hardware.h"

#ifndef __EMSCRIPTEN__
#include "p2p.h"
#include "ini.h"
#endif

#include <stdint.h>
#include <string.h>

uint8_t SB_REG;
uint8_t SC_REG;

#ifndef __EMSCRIPTEN__
P2P_connection p2p;
#endif

size_t serial_counter = 0;
enum SERIAL_MODE { SLAVE, MASTER }  serial_mode;

void initSerial(){
    #ifdef __EMSCRIPTEN__
    return;
    #else
    P2P_init(&p2p);
    if(!load_network_config()){
        return;
    }

    SDLNet_Init();

    P2P_establishConnection(&p2p);

    serial_mode = SLAVE;
    #endif
}

void freeSerial(){
    #ifndef __EMSCRIPTEN__
    P2P_close(&p2p);
    #endif
}

void updateSerial(){
    if(serial_counter--)
        return;

    serial_counter = 4096;

    #ifndef __EMSCRIPTEN__
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
    } else 
    #endif
    {
        if((SC_REG & 0x80) && (SC_REG & 0x01)){
            SB_REG = 0xFF;
            SC_REG &= 0x7F;
            cpu.IF |= SERIAL_IRQ;
        }  
    }      
}

#ifndef __EMSCRIPTEN__

bool load_network_config(){
    if(config_listen_port)
        p2p.my_server_port = config_listen_port;
    else
        return false;

    if(config_connect_port)
        p2p.peer_server_port = config_connect_port;
    else
        return false;

    if(config_connect_ip[0])
        strcpy(p2p.peer_server_ip, config_connect_ip);
    else
        return false;

    return true;
}

#endif