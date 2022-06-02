#define _WINSOCK_DEPRECATED_NO_WARNINGS        // turns of deprecated warnings for winsock
#define _CRT_SECURE_NO_WARNINGS

#define BUFFERSIZE 1024

#pragma once
#include <winsock2.h>
#include <iostream>
#include "Helper.h"
#include "Definitions.h"

#pragma comment(lib,"Ws2_32.lib")

class ClientUDP
{
private:
	SOCKET socketUDP;
	sockaddr_in serverAddr;
	u_long optVal = 1;
	char broadVal = '1';
	char* buffer = nullptr;
	int addrLength = sizeof(serverAddr);


public:
	u_long GetServerAddress(void) const { return serverAddr.sin_addr.S_un.S_addr; }
	int CreateSocket(uint16_t port);
};

