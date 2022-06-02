#include "../Header Files/Helper.h"

uint16_t Helper::GetValidatedPortNumber()
{
	uint16_t userInput;
	std::cout <<  "Enter the server's port number: ";
	while (true)
	{
		std::cin >> userInput;
		if (std::cin.fail() || (userInput < 0 || userInput > 65536))
		{
			std::cin.clear();
			std::cin.ignore(UINT16_MAX, '\n');
			std::cout << std::endl << "You did not enter a valid port number, please try again: " << std::endl;
			continue;
		}
		std::cin.ignore(UINT16_MAX, '\n');
		break;
	}
	return userInput;
}

bool Helper::AddressCorrectFormat(char* address) {
	int dots = 0;
	for (int i = 0; address[i] != '\0'; i++) {
		if (address[i] == '.') {
			dots++;
		}
	}
	if (dots == 3) {
		return true;
	}
	else {
		return false;
	}
}

void Helper::UserInput(const char* message, char* buffer, int size) {
	std::cout << message;
	std::cin.getline(buffer, size);
}

int Helper::MessageSize(char* message) {
	int total;
	for (total = 0; message[total] != '\0'; total++);
	return total;
}

char* Helper::CopyMessage(char* bufferToCopy, int size) {
	char* toReturn = new char[size + 1];
	for (int i = 0; i < size; i++) {
		toReturn[i] = bufferToCopy[i];
	}
	toReturn[size] = '\0';
	return toReturn;
}

bool Helper::CompareUserInput(const char* _string1, char* _string2) {
	int size1;
	int size2;
	
	for (size1 = 0; _string1[size1] != '\0'; size1++);
	for (size2 = 0; _string2[size2] != '\0'; size2++);
	
	if (size1 != size2) {
		return false;
	}

	for (int i = 0; i < size1; i++) {
		if (_string1[i] != _string2[i]) {
			return false;
		}
	}

	return true;
 }

int Helper::ResultType(int _error) {
	switch (_error) {
	case -1:
		std::cout << "The connection has been disconnected." << std::endl;
		return MESSAGE_ERROR;
	case 0:
		std::cout << "There was an issue with the message.  It didn't return anytihng.  Please check the message and try again." << std::endl;
		return MESSAGE_ERROR;
	case 1:
		std::cout << "The connection has been shutdown." << std::endl;
		return SHUTDOWN;
	case 2:
		std::cout << "The connection has been disconnected." << std::endl;
		return DISCONNECT;
	case 3:
		std::cout << "There was an issue binding the socket." << std::endl;
		return BIND_ERROR;
	case 4:
		std::cout << "There was an issue connecting.  Please try again later." << std::endl;
		return CONNECT_ERROR;
	case 5:
		std::cout << "There was an setting up the connection.  Please try again later." << std::endl;
		return SETUP_ERROR;
	case 6:
		std::cout << "There was an issue starting up.  Please try  again later." << std::endl;
		return STARTUP_ERROR;
	case 7:
		std::cout << "The was an issue with the address. Please make sure it is valid and in the quadratic format." << std::endl;
	case 8: 
		std::cout << "There was an issue with the message.  It was either too long or not long enough." << std::endl;
		return PARAMETER_ERROR;
	case 9:
		std::cout << "There was an issue with the message.  Please check the message and try again." << std::endl;
		return MESSAGE_ERROR;
	case 10:
		return SV_SUCCESS;
	case 11:
		return SV_FULL;
	case 10004:
		std::cout << "There was an issue with the connection and has been shutdown" << std::endl;
		return SHUTDOWN;
	case 10054:
		std::cout << "There has been an issue with the connection and has been disconnected" << std::endl;
		return DISCONNECT;
	case 10058:
		std::cout << "The connection has been shutdown." << std::endl;
		return SHUTDOWN;
	case 10061:
		std::cout << "There was an issue with connecting to the sever.  It may not be active.  Please try again later." << std::endl;
		return CONNECT_ERROR;
	default:
		return SUCCESS;
	}
}

long long Helper::GetFileSize(char* buffer) {
	long long result = 0;
	buffer = CopyMessage(buffer, MessageSize(buffer));
	for (int i = 0; buffer[i] >= 48; i++) {
		result += (long long)buffer[i] - 48;
		result *= 10;
	}
	//The loop won't end until it verifies the end of the string.  This adds an extra multiple of 10, so it needs to be removed.
	result /= 10;
	return result;
}

void Helper::RecieveIP(char* data, char* ip) {
	bool ipData = true;
	for (int i = 0; i < Helper::MessageSize(data); i++) {
		if (data[i] == ',')
		{
			ip[i] = '\0';
			break;
		}

		if (ipData == true) {
			ip[i] = data[i];
		}
		//else {
		//	port[i] = data[i];
		//}
	}
}

void Helper::RecievePort(char* data, uint16_t* port) {
	bool ipData = true;
	uint16_t num = 0;
	for (int i = 0; i < Helper::MessageSize(data); i++) {
		num *= 10;
		if (data[i] == '\0') {
			break;
		}
		else if (data[i] == ',') {
			ipData = false;
			continue;
		}

		if(ipData == false) {
			num += data[i] - 48;
		}
	}
	*port = (uint16_t)num;
}