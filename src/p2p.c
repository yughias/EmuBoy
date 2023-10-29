#include "p2p.h"

#include <stdio.h>
#include <string.h>

TCPsocket createSocket(const char*, Uint16);

void P2P_init(P2P_connection* p2p){
    memset(p2p, 0, sizeof(P2P_connection));
}

void P2P_establishConnection(P2P_connection* p2p){
    printf("establishing p2p...\n");

    p2p->my_server_sock = createSocket("0.0.0.0", p2p->my_server_port);
    p2p->peer_set = SDLNet_AllocSocketSet(1);

    while(!p2p->connection_established){
        if(!p2p->peer_connection_sock){
            p2p->peer_connection_sock = SDLNet_TCP_Accept(p2p->my_server_sock);
            if(p2p->peer_connection_sock){
                SDLNet_TCP_AddSocket(p2p->peer_set, p2p->peer_connection_sock);
                printf("peer connected to me!\n");
            }
        }

        if(!p2p->peer_server_sock){
            p2p->peer_server_sock = createSocket(p2p->peer_server_ip, p2p->peer_server_port);
            if(p2p->peer_server_sock)
                printf("connected to peer server!\n");
        }

        if(p2p->peer_server_sock && p2p->peer_connection_sock)
            p2p->connection_established = true;
    }

    printf("p2p established!\n");
}

int P2P_send(P2P_connection* p2p, void* data, int len){
    return SDLNet_TCP_Send(p2p->peer_server_sock, data, len);
}

int P2P_no_blocking_recv(P2P_connection* p2p, void* data, int len){
    if(SDLNet_CheckSockets(p2p->peer_set, 0)){
            return SDLNet_TCP_Recv(p2p->peer_connection_sock, data, len);
    }
    return 0;
}

int P2P_blocking_recv(P2P_connection* p2p, void* data, int len){
    return SDLNet_TCP_Recv(p2p->peer_connection_sock, data, len);
}

void P2P_close(P2P_connection* p2p){
    SDLNet_TCP_Close(p2p->peer_server_sock);
    SDLNet_TCP_Close(p2p->peer_connection_sock);
    SDLNet_TCP_Close(p2p->my_server_sock);
    SDLNet_FreeSocketSet(p2p->peer_set);
    P2P_init(p2p);
}

TCPsocket createSocket(const char* ip_addr, Uint16 port){
    IPaddress ip;
    SDLNet_ResolveHost(&ip, ip_addr, port);
    return SDLNet_TCP_Open(&ip);
}