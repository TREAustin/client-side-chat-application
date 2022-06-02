#define _WINSOCK_DEPRECATED_NO_WARNINGS        // turns of deprecated warnings for winsock
#define _CRT_SECURE_NO_WARNINGS
#define BUFFERSIZE 1024

#pragma once
#include <winsock2.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Helper.h"
#include "Definitions.h"

class ClientTCP
{
private:
	char* buffer = nullptr;
	bool registered = false;
	char* ip = nullptr;
	uint16_t portNum = 0;
	const char* currUser = "";
	char broadVal = 1;
	struct timeval tv; 
	u_long value = 1;	
	int addrLength = 0;
	fd_set masterSet, readSet;
	std::string filePath;
	std::ostringstream oss;
	SOCKET broadcastSocket;
	SOCKET commSocket;
	sockaddr_in serverAddr, broadAddr;
	int broadLength = sizeof(broadAddr);
public:
	bool GetStatus() const { return registered; }
	void SetStatus(bool status) { registered = status; }
	char* GetIPInfo() const { return ip; }
	uint16_t GetPortInfo() const { return portNum; }
	int initUDP();
	int init(char* username);
	int $register(char* username);
	int readMessage(char* buffer, int32_t size);
	int sendMessage(char* data, int32_t length);
	int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
	int tcp_recv_whole(SOCKET s, char* buf, int len);
	int recv_udp(char* buffer, int32_t size);
	void stop();
	void logEvent(const char* _event, int type);
};