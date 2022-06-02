#include "../Header Files/ClientTCP.h"

int ClientTCP::initUDP() {
	int result = 0;
	buffer = new char[BUFFERSIZE];

	broadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (broadcastSocket == INVALID_SOCKET)
	{
		return Helper::ResultType(5);
	}

	result = setsockopt(broadcastSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&broadVal, sizeof(broadVal));

	broadAddr.sin_addr.s_addr = INADDR_ANY;
	broadAddr.sin_family = AF_INET;
	broadAddr.sin_port = htons(31337);

	result = bind(broadcastSocket, (SOCKADDR*)&broadAddr, sizeof(broadAddr));
	if (result == SOCKET_ERROR)
	{
		return Helper::ResultType(3);
	}

	result = recvfrom(broadcastSocket, buffer, BUFFERSIZE, 0, (SOCKADDR*)&broadAddr, &broadLength);
	if (result < 0 || WSAGetLastError() != 0) {
		return Helper::ResultType(2);
	}

	char* tempBuffer = Helper::CopyMessage(buffer, result);
	ip = new char[16];
	Helper::RecieveIP(tempBuffer, ip);
	Helper::RecievePort(tempBuffer, &portNum);
	std::cout << "IP Address: " << ip << ", Port: " << portNum << std::endl;

	return result;
}


int ClientTCP::init(char* username)
{
	//Result to be returned.
	int result = SUCCESS;
	//Open commSocket for the server.
	commSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (commSocket == INVALID_SOCKET)
	{
		//Will return CONNECT_ERROR
		return Helper::ResultType(5);
	}

	//Handles the addressing.127
	serverAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNum);

	//Connect to the server.
	result = connect(commSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	//If a WSA Error was thrown, this will handle the appropriate message.
	if (WSAGetLastError() != 0) {
		return Helper::ResultType(WSAGetLastError());
	}
	//Register the user.
	result = $register(username);
	filePath.append(username);
	filePath.append("_log.txt");
	logEvent("Connected to server.", 0);
	return result;
}

int ClientTCP::$register(char* username) {
	//Determines size of the username and adds a '\0' character at the end.
	int size = Helper::MessageSize(username);
	//Sends username to server for registering client.
	int result = sendMessage(username, size);
	//If a WSA Error was thrown, this will handle the appropriate message.
	if (WSAGetLastError() != 0) {
		return Helper::ResultType(WSAGetLastError());
	}
	//Recieves message from the server, returns SV_SUCCESS or SV_FULL.
	result = readMessage(buffer, 1024);
	//If a WSA Error was thrown, this will handle the appropriate message.
	if (WSAGetLastError() != 0) {
		return Helper::ResultType(WSAGetLastError());
	}
	if (Helper::CompareUserInput("SV_FULL", buffer)) {
		//Disconnects and shuts down the socket if the server is full.
		stop();
		//Handles SV_FULL
		return Helper::ResultType(11);
	}
	else {
		//If "SV_SUCCESS" is returned, the bool registered is set to true.
		registered = true;
		return SV_SUCCESS;
	}
	//No return since it is handled in the last two if/else statements.
}

int ClientTCP::readMessage(char* _buffer, int32_t size)
{
	//Variable is used to store size since BUFFERSIZE has been defined.
	int tempSize = 0;
	//Recieve message size.
	int result = tcp_recv_whole(commSocket, (char*)&tempSize, 1);
	//If the socket is -1 or has no size, this will return the approriate message.
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		//Determines the error and returns the appropriate error type.
		return -1;
	}
	//If a WSA Error was thrown, this will handle the appropriate message.
	else if (WSAGetLastError() != 0) {
		//Will return the error type associated with the connection error.
		return Helper::ResultType(WSAGetLastError());
	}
	//Recieve the message.
	result = tcp_recv_whole(commSocket, _buffer, tempSize);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		//Determines the error and returns the appropriate error type.
		return Helper::ResultType(result);
	}
	//If a WSA Error was thrown, this will handle the appropriate message.
	else if (WSAGetLastError() != 0) {
		//Will return the error type associated with the connection error.
		return Helper::ResultType(WSAGetLastError());
	}
	else if (result != tempSize) {
		//Returns the PARAMETER_ERROR.
		return Helper::ResultType(8);
	}

	return result;
}

int ClientTCP::sendMessage(char* data, int32_t length)
{
	//Sends message size.
	int result = tcp_send_whole(commSocket, (char*)&length, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		//Determines the error and returns the appropriate error type.
		return Helper::ResultType(WSAGetLastError());
	}
	//Sends the message.
	result = tcp_send_whole(commSocket, data, Helper::MessageSize(data));
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		//Determines the error and returns the appropriate error type.
		return Helper::ResultType(WSAGetLastError());
	}
	else if (length < 0 || length > 1024) {
		//Returns the PARAMETER_ERROR.
		return Helper::ResultType(8);
	}

	return result;
}

void ClientTCP::stop()
{
	//Shut down socket.
	shutdown(commSocket, SD_BOTH);
	//Close socket.
	closesocket(commSocket);
	//Delete buffer from heap.
	delete[] buffer;
	//Set buffer equal to nullptr.
	buffer = nullptr;
}

int ClientTCP::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}

int ClientTCP::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;

	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);

	return total;
}

int ClientTCP::recv_udp(char* buffer, int32_t size) {
	int result = 0;
	result = recvfrom(broadcastSocket, buffer, BUFFERSIZE, 0, (SOCKADDR*)&broadAddr, &broadLength);
	if (size < 0 || WSAGetLastError() != 0) {
		return Helper::ResultType(2);
	}
	char* tempBuffer = Helper::CopyMessage(buffer, result);
	Helper::RecieveIP(tempBuffer, ip);
	Helper::RecievePort(tempBuffer, &portNum);
	//I wasn't sure if I needed to print the broadcast or not.  If I did, the line below will do that.
	//std::cout << "IP Address: " << ip << " | Port: " << portNum << std::endl << " >";
	return result;

}

void ClientTCP::logEvent(const char* _event, int type) {
	FILE* events;
	//Type 0 used for server being initialized.
	if (type == 0) {
		events = fopen(filePath.c_str(), "w");
		oss.str("");
	}
	//Type 1 used for appending.
	else {
		events = fopen(filePath.c_str(), "a");
	}
	if (events != NULL) {
		fputs(_event, events);
		fclose(events);
	}
	oss.str("");
}
