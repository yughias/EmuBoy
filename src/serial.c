#include "SDL_MAINLOOP.h"
#include "serial.h"
#include "gb.h"

#ifndef __EMSCRIPTEN__
#include "p2p.h"
#include "ini.h"
#endif

#include <stdint.h>
#include <string.h>

#ifndef __EMSCRIPTEN__
P2P_connection p2p;
#endif

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
    #endif
}

void freeSerial(){
    #ifndef __EMSCRIPTEN__
    P2P_close(&p2p);
    #endif
}

void updateSerial(gb_t* gb){
    sm83_t* cpu = &gb->cpu;
    serial_t* serial = &gb->serial;
    if(serial->counter--)
        return;

    serial->counter = 4096;

    #ifndef __EMSCRIPTEN__
    if(p2p.connection_established){
        switch(serial->mode){
            case SLAVE:
            uint8_t out_byte = serial->SB_REG;
            if((serial->SC_REG & 0x80) && !(serial->SC_REG & 0x01) && P2P_no_blocking_recv(&p2p, &serial->SB_REG, 1)){
                serial->SC_REG &= 0x7F;
                cpu->IF |= SERIAL_IRQ;
                P2P_send(&p2p, &out_byte, 1);  
                return;
            }

            if((serial->SC_REG & 0x80) && (serial->SC_REG & 0x01)){
                P2P_send(&p2p, &serial->SB_REG, 1);
                serial->mode = MASTER;
                return;
            }
            break;

            case MASTER:
            P2P_blocking_recv(&p2p, &serial->SB_REG, 1);
            serial->SC_REG &= 0x7F;
            cpu->IF |= SERIAL_IRQ;
            serial->mode = SLAVE;  
            break;
        }
    } else 
    #endif
    {
        if((serial->SC_REG & 0x80) && (serial->SC_REG & 0x01)){
            serial->SB_REG = 0xFF;
            serial->SC_REG &= 0x7F;
            cpu->IF |= SERIAL_IRQ;
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