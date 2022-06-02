#include <iostream>
#include "Definitions.h"
#pragma once

namespace Helper
{
	uint16_t GetValidatedPortNumber();
	bool AddressCorrectFormat(char* address);
	void UserInput(const char* message, char* buffer, int size);
	int MessageSize(char* message);
	char* CopyMessage(char* bufferToCopy, int size);
	bool CompareUserInput(const char* _string1, char* _string2);
	int ResultType(int _error);
	long long GetFileSize(char* buffer);
	void RecieveIP(char* data, char* ip);
	void RecievePort(char* data, uint16_t*);
};

