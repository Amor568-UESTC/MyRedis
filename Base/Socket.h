#pragma once

#include<arpa/inet.h>
#include<string.h>
#include<string>
#include<memory>
#include<atomic>
#include<functional>

#define INVALID_SOCKET (int)(~0)
#define SOCKET_ERROR (-1)
#define INVALID_PORT (-1)

struct SocketAddr
{
    sockaddr_in addr_;

    SocketAddr() { Clear();}
    SocketAddr(const SocketAddr& other) { memcpy(&addr_,&other.addr_,sizeof(addr_));}
    
}