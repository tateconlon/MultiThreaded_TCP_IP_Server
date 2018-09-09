#include "stdafx.h"
#include "TcpipServer.h"
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

//TODO: clean up all "new" threads from heap
//TODO: Seperate into UI class
//TODO: Ask for name

void TcpipServer::runUIThread() {
	std::thread* ui = new std::thread(&TcpipServer::UI, this);
	ui->detach();
}

void TcpipServer::UI() {
	lastReadChar = '\0';
	while (true) {
		std::cin >> lastReadChar;
	}
}

void TcpipServer::runServer()
{
	runUIThread();
	//Initialize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		std::cerr << "Can't initialize wonsock! Quitting" << std::endl;
		return;
	}

	//----- Create a socket
	//AF_INET means TCP or UDP
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0); //socket is TCP

														//Socket's are glorified integers
	if (listening == INVALID_SOCKET) {
		std::cerr << "Can't create a socket. Quitting" << std::endl;
		return;
	}

	//----- Bind the socket to an ip address and port

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	//htons changes the 54000 to big endian, network addressing is big endian, pc addressing is little endian
	hint.sin_port = htons(54000);
	//gives port ip address
	hint.sin_addr.S_un.S_addr = INADDR_ANY;	//i dunno

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	//listening is now our file descriptor!

	//-----Tell winsock the socket is for listening
	listen(listening, SOMAXCONN);	//max connections you can backlog


	//-----Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	std::thread* tempPtr;

	timeval timeout;
	ZeroMemory(&timeout, sizeof(timeout));
	timeout.tv_sec= 1;

	int selectRetVal = 0;

	while(true) {
		//Wait until something tries to connect, server specific
		//TODO: check if numConnections is 0. Needs its own thread???

		selectRetVal = 0;
		while (selectRetVal <= 0 && lastReadChar != 'q') {
			fd_set readfds;	//fd stands for file descriptor
			//readfds.fd_count = 1;
			//readfds.fd_array[0] = listening;	//Might not be portable. Use FD_SET??? FD_ISSET?

			FD_ZERO(&readfds);
			FD_SET(listening, &readfds);

			selectRetVal = select(listening+1, &readfds, NULL, NULL, &timeout);	//TODO: handle -1 case
			std::cout << "~";
		}

		if (lastReadChar == 'q') {
			break;	//leave loop
		}

		SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);	//TODO: O_NONBLOCK ???

		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "client was invalid. Quitting" << std::endl;
			continue;
		}

		ConnectArgs temp(clientSocket, client, clientSize);

		//connect(clientSocket, client, clientSize);

		void(TcpipServer::*properFunction)(ConnectArgs) = &TcpipServer::connect;

		//(this->*properFunction)(temp);

		//std::thread t1(properFunction, this, temp));

		////t1.detach();
		//allThreads.push_back();
		//std::this_thread::sleep_for(std::chrono::seconds(5)); //delay deletion
		//std::cout << "deleted" << std::endl;

		//std::thread t2 = std::thread(properFunction, this, temp);

		//tempPtr = &t2;//new std::thread(properFunction, this, temp);
		//tempPtr->detach();


		//TODO: Ask stackoverflow if this is safe? Terminating a function frees resources, will it free on heap?
		std::thread* tempThread = new std::thread(properFunction, this, temp);
		tempThread->detach();

		//Because we are detaching threads, we do not need to keep track of their ID.
		//They will destroy themselves on their own as they finish executing (???)
		//allThreads.push_back(tempPtr);

	}

	//Join threads here?

	//Cleanup winsock
	WSACleanup();
}

void TcpipServer::connect(SOCKET clientSocket, sockaddr_in client, int clientSize) {
	int nameInd = allSockets.size();	//TODO: not correct, should be a queue or something, where you can't have one already taken up
	//If you're 0 then another one connects ([1]) then you disconnect then reconnect then you will also have [1].
	allSockets.push_back(clientSocket);

	std::cout << names[nameInd].c_str() << " connected!" << std::endl;
	std::cout << allSockets.size() << " clients connected!" << std::endl << std::endl;

	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	//clear arrays to 0
	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	//DNS lookup? Gets information about the client!
	char* serviceStr;
	serviceStr = service;
	if (getnameinfo((sockaddr*)&client, sizeof(client),
		host, NI_MAXHOST,
		service, NI_MAXSERV, 0) == 0) {
		//Success!
		std::cout << host << " connected on port " << service << std::endl;
		serviceStr = service;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " <<
			ntohs(client.sin_port) << std::endl;
	}
	//ntop -> network to presentation, ip address
	//ntoh -> network to host, port
	//Always need to convert! (probably)

	//---- while: accept and echo
	char buf[4096];

	while (true) {
		ZeroMemory(buf, 4096);

		//wait for client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);

		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Error" << std::endl;
			break;
		}

		if (bytesReceived == 0) {
			std::cout << "Client disconnected" << std::endl;
			break;
		}

		if (buf[0] == 'q') {
			break;
		}

		//if (buf[bytesRecieved - 1] == '\n') {
		//	std::cout << "Newline recieved" << std::endl;
		//}
		//else if (buf[bytesRecieved - 1] == '\0') {
		//	std::cout << "End of string recieved" << std::endl;
		//}



		//TODO: Not thread safe???

		//When a client sends a message, it sends twice. The first message contains all the text,
		//And the second sends just the /r/n. When echoing to the all the other clients, the /n/r doesn't need
		//the name in front of it.
		for (std::vector<SOCKET>::iterator i = allSockets.begin(); i != allSockets.end(); i++) {
			if ((*i) != clientSocket) {
				char* bytesToSend;
				int bytesToSendLength;
				bool needsToBeDeleted = false;
				if (buf[0] == '\r' && buf[1] == '\n' && bytesReceived == 2) {
					bytesToSend = buf;
					bytesToSendLength = bytesReceived + 1;
				}
				else {
					bytesToSendLength = bytesReceived + names[nameInd].length() + 1;
					bytesToSend = new char[bytesToSendLength];	//cannot dynamically allocate arrays without chars on stack, must use new and clean up
					needsToBeDeleted = true;

					const char* nameStr = names[nameInd].c_str();
					const char* newBuf = buf;
					std::copy(nameStr, nameStr + names[nameInd].length(), bytesToSend);
					std::copy(newBuf, newBuf + bytesReceived + 1, bytesToSend + names[nameInd].length());

				}

				if (needsToBeDeleted) {
					delete[] bytesToSend;
				}
			}
		}
		

	}

	//Close socket
	closesocket(clientSocket);

	//REMEMBER: sends twice, first everything but the newline, then the newline.

	//TODO: Semaphore since other threads might also be deleting this????
	//TODO: Just detach the threads???
	//TODO: No need to delete threads as their resources are freed on termination (even if they are on the heap)?
	//Instead will just keep track of number of open connections
	/*for (std::vector<std::thread*>::iterator i = allThreads.begin(); i != allThreads.end(); i++) {
		std::cout << (*i)->get_id() << " | " << std::this_thread::get_id() << std::endl;
		if ((*i)->get_id() == std::this_thread::get_id()) {
			allThreads.erase(i);
			delete *i;
			std::cout << "Does this print?" << std::endl;
			std::cout << "connection gone: threads left = " << allThreads.size() << std::endl;
			break;
		}
	}*/

	//TODO: not thread safe?
	for (std::vector<SOCKET>::iterator i = allSockets.begin(); i != allSockets.end(); i++) {
		if ((*i) == clientSocket) {
			i = allSockets.erase(i);
			break;
		}
	}

	std::cout << names[nameInd].c_str() << " disconnected!" << std::endl;
	std::cout << allSockets.size() << " clients now connected." << std::endl << std::endl;
}

void TcpipServer::connect(ConnectArgs c)
{
	connect(c.clientSocket, c.client, c.clientSize);
}

TcpipServer::TcpipServer() : log("chats")
{
	//TODO: how to string into char buffer with \0 (Joel Splosky's blog post?)
	std::string t = "Tate: ";
	std::string a = "Ameris: ";
	std::string n = "Neil: ";
	std::string ad = "Adan: ";
	std::string r = "Riley: ";
	names.push_back(t);
	names.push_back(a);
	names.push_back(n);
	names.push_back(ad);
	names.push_back(r);
}


TcpipServer::~TcpipServer()
{
}
