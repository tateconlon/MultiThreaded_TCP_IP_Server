#pragma once

#include <WS2tcpip.h>
#include <vector>
#include <thread>
#include "LogFile.h"

class TcpipServer
{
public:
	struct ConnectArgs {
		SOCKET clientSocket;
		sockaddr_in client;
		int clientSize;

		ConnectArgs(SOCKET clientSocket, sockaddr_in client, int clientSize) {
			this->clientSocket = clientSocket;
			this->client = client;
			this->clientSize = clientSize;
		}
	};
	TcpipServer();
	~TcpipServer();
	void runUIThread();
	void UI();
	void runServer();
	void connect(SOCKET clientSocket, sockaddr_in client, int clientSize);
	void connect(ConnectArgs c);
	void nothing() { std::this_thread::sleep_for(std::chrono::seconds(100)); }
	//void nothing(char c) { c; }
	std::vector<std::thread*> allThreads;
	std::vector<std::string> names;//, 		//TODO: more names obvs
private:
	std::vector<SOCKET> allSockets;
	char lastReadChar;
	LogFiles log;
};



