#include "../Header Files/ClientUDP.h"

int ClientUDP::CreateSocket(uint16_t port) {
	int result = SUCCESS;
	buffer = new char[BUFFERSIZE];
	socketUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	result = bind(socketUDP, (SOCKADDR*)&serverAddr, addrLength);

	result = ioctlsocket(socketUDP, FIONBIO, &optVal);
	result = setsockopt(socketUDP, SOL_SOCKET, SO_BROADCAST, &broadVal, sizeof(optVal));

	result = sendto(socketUDP, "Hey there", 10, 0, (SOCKADDR*)&serverAddr, addrLength);
	std::cout << (char*)result << std::endl;
	result = recvfrom(socketUDP, buffer, BUFFERSIZE, 0, (SOCKADDR*)&serverAddr, &addrLength);

	std::cout << Helper::CopyMessage(buffer, Helper::MessageSize(buffer)) << std::endl;
	closesocket(socketUDP);
	delete[] buffer;

	return result;
}