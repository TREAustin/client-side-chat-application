#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock

#include <winsock2.h>
#include <iostream>
#include "../Header Files/Helper.h"
#include "../Header Files/ClientTCP.h"
#include "../Header Files/ClientUDP.h"
#include "../Header Files/Helper.h"
#include <thread>
#include <mutex>
#include <vector>

//#include <ws2tcpip.h>                         // only need if you use inet_pton
#pragma comment(lib,"Ws2_32.lib")

struct ThreadStruct {
	ClientTCP* client;
	std::mutex* mut;
	std::condition_variable* cv;
	int* numOfThreads;
	bool* input;
};

void EnterUsername(char* username);
int FirstScreen(ClientTCP* _client, char* _buffer, uint16_t _port, char* _ipAddress, char* _username, bool* input);
void SecondScreen(ClientTCP* client, char* _buffer, std::mutex* mut, std::condition_variable* cv, int* numOfThreads, bool* input);
void MonitorBroadcastMessage(ThreadStruct* threadData);
void MonitorServerMessage(ThreadStruct* threadData);
void MonitorUserInput(ThreadStruct* threadData);
void GetLog(ClientTCP* _client);

int main() {
	WSADATA wsadata;
	int wsa = WSAStartup(WINSOCK_VERSION, &wsadata);
	int numOfThreads = 3;
	int result = 0;

	std::mutex mut;
	std::condition_variable cv;
	std::thread broadcastThread;
	
	bool input = false;
	//Initiazed ClientTCP Object.
	ClientTCP* client = new ClientTCP();
	//Buffer to be used for IP Address.
	char* ipAddress = new char[15];
	//Buffer to be used for username.
	char* username = new char[30];
	//Unsigned 16 bit int to be used for port, limits the max to 65535.
	uint16_t port = 0;
	//Buffer to be used for messaging.
	char* buffer = new char[1024];

	std::cout << "Waiting for Server...." << std::endl;
	result = client->initUDP();
	EnterUsername(username);
	if (result != 0) {
		Helper::ResultType(4);
	}
	ThreadStruct udp;
	udp.client = client;
	udp.cv = &cv;
	udp.mut = &mut;
	udp.numOfThreads = &numOfThreads;
	udp.input = &input;
	
	//Register Client loop.  If user does not register, then program will exit.
	result = FirstScreen(client, buffer, port, ipAddress, username, &input);
	//If the cient registered, then print the second screen menu.
	std::thread udpConnection(MonitorBroadcastMessage, &udp);
	udpConnection.detach();
	if(client->GetStatus()){
		std::cout << "-------------------------------------------------" << std::endl;
		std::cout << "Welcome: " << username << std::endl;
		std::cout << "Options: " << std::endl;
		std::cout << "Either enter the message you want to send or type any of the following commands" << std::endl;
		std::cout << "$getlist - Retrieve all users connected." << std::endl;
		std::cout << "$getlog - Retrieve the log activity" << std::endl;
		std::cout << "$exit" << std::endl;
		SecondScreen(client, buffer, &mut, &cv, &numOfThreads, &input);
	}

	//Clean up buffers.
	std::unique_lock<std::mutex> lock(mut);
	cv.wait(lock, [&]() { return numOfThreads == 0; });
	Helper::UserInput("Press Enter to exit.", buffer, 2);

	delete[] ipAddress, username, buffer;
	return WSACleanup();
}

void EnterUsername(char* username) {
	//Method for handling getting username.
	Helper::UserInput("\nEnter your username: \n>", username, 30);
}

int FirstScreen(ClientTCP* _client, char* _buffer, uint16_t _port, char* _ipAddress, char* _username, bool* input) {
	system("cls");
	int result = SUCCESS;
	while (!_client->GetStatus()) {
		std::cout << "--- Welcome to the chat app ---" << std::endl;
		std::cout << "Type the command to continue: " << std::endl;
		std::cout << "1.) $register" << std::endl;
		std::cout << "2.) $exit" << std::endl;
		Helper::UserInput	("> ", _buffer, 1024);
		if (Helper::CompareUserInput((char*)"$register", _buffer)) {
			std::cout << std::endl << "Registering: " << _username << std::endl;
			int result = _client->init(_username);
			if (result != SV_SUCCESS)
			{
				std::cout << "The server is currently full.  Please try again later." << std::endl;
				*input = true;
				return result;
			}
		}
		else {
			*input = true;
			_client->stop();
			return result;
		}
	}
	system("cls");
	return result;
}

void SecondScreen(ClientTCP* client, char* _buffer, std::mutex* mut, std::condition_variable* cv, int* numOfThreads, bool* input) {
	std::thread* _userInput;
	std::thread* _serverMessages;
	ThreadStruct* threadData = new ThreadStruct[2];
	
	for (int i = 0; i < 2; i++) {
		threadData[i].client = client;
		threadData[i].mut = mut;
		threadData[i].cv = cv;
		threadData[i].numOfThreads = numOfThreads;
		threadData[i].input = input;
	}
	std::thread server(MonitorServerMessage, &threadData[0]);
	std::thread user(MonitorUserInput, &threadData[1]);

	server.detach();
	user.detach();
}

void MonitorBroadcastMessage(ThreadStruct* threadData) {
	int size = 0;
	char* messageBuffer = new char[BUFFERSIZE];
	char* tempBuffer;
	while (threadData->client->GetStatus()) {
		size = threadData->client->recv_udp(messageBuffer, Helper::MessageSize(messageBuffer));
		std::unique_lock<std::mutex> lock(*(threadData->mut));
		if (size < 0 || WSAGetLastError() != 0 && *(threadData->input) == false) {
			*(threadData->input) = true;
			Helper::ResultType(2);
			threadData->client->SetStatus(false);
			break;
		}
		threadData->cv->wait(lock, []() { return true; });
	}
	if (*threadData->numOfThreads != 0) {
		*threadData->numOfThreads -= 3;
	}
	threadData->cv->notify_all();
}

void MonitorServerMessage(ThreadStruct* threadData) {
	int size = 0;
	char* messageBuffer = new char[BUFFERSIZE];
	char* tempBuffer;
	while (threadData->client->GetStatus()) {
		size = threadData->client->readMessage(messageBuffer, Helper::MessageSize(messageBuffer));
		std::unique_lock<std::mutex> lock(*(threadData->mut));
		if ((size < 0 || WSAGetLastError()) != 0 && *(threadData->input) == false) {
			*(threadData->input) = true;
			Helper::ResultType(2);
			threadData->client->SetStatus(false);
			break;
		}
		tempBuffer = Helper::CopyMessage(messageBuffer, size);
		if (Helper::CompareUserInput("$getlog", tempBuffer)) {
			std::cout << "Current Log: " << std::endl;
			GetLog(threadData->client);
		}
		else {
			std::cout << tempBuffer << "> ";
		}
		threadData->cv->wait(lock, []() { return true; });
	}
	if (*threadData->numOfThreads != 0) {
		*threadData->numOfThreads -= 3;
	}
	threadData->cv->notify_all();
}

void MonitorUserInput(ThreadStruct* threadData) {
	char* messageBuffer = new char[BUFFERSIZE];
	char* tempBuffer;
	while (threadData->client->GetStatus()) {
		Helper::UserInput("> ", messageBuffer, BUFFERSIZE);
		std::unique_lock<std::mutex> lock(*(threadData->mut));
		messageBuffer = Helper::CopyMessage(messageBuffer, Helper::MessageSize(messageBuffer));
		threadData->client->sendMessage(messageBuffer, Helper::MessageSize(messageBuffer));
		if (Helper::CompareUserInput("$exit", messageBuffer)){
			*(threadData->input) = true;
			threadData->client->SetStatus(false);
			break;
		}
		threadData->cv->wait(lock, []() { return true; });
	}
	if (*threadData->numOfThreads != 0) {
		*threadData->numOfThreads -= 3;
	}
	threadData->cv->notify_all();
}

void GetLog(ClientTCP* _client) {
	int result = 0;
	char* _buffer = new char[BUFFERSIZE];
	result = _client->readMessage(_buffer, BUFFERSIZE);
	long long fileSize = Helper::GetFileSize(_buffer);
	while (fileSize != 0) {
		result = _client->readMessage(_buffer, BUFFERSIZE);
		char* tempBuffer = Helper::CopyMessage(_buffer, result);
		std::cout << " " << tempBuffer;
		_client->logEvent(tempBuffer, 1);
		fileSize -= (result + (long long)1);
	}
	std::cout << "> ";
}