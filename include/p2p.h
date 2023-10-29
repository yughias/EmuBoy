#ifndef __P2P_H__
#define __P2P_H__

#include <SDL2/SDL_net.h>
#include <stdbool.h>

typedef struct {
    Uint16 my_server_port;
    Uint16 peer_server_port;
    char peer_server_ip[16];

    TCPsocket my_server_sock;
    TCPsocket peer_connection_sock;
    TCPsocket peer_server_sock;
    SDLNet_SocketSet peer_set;

    bool connection_established;
} P2P_connection;

void P2P_init(P2P_connection*);
void P2P_establishConnection(P2P_connection*);
int P2P_send(P2P_connection*, void*, int);
int P2P_no_blocking_recv(P2P_connection*, void*, int);
int P2P_blocking_recv(P2P_connection*, void*, int);

void P2P_close(P2P_connection*);

#endif