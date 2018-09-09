// TCP-IP server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <thread>
#include "TcpipServer.h"

void spam();
char spamChar = '.';

void function_1() {
	std::thread spamThread(spam);
	//spamThread.detach();
	char c;
	std::cout << "Beauty is only skin-deep\n";// << std::endl;
	spamThread.join();
	//std::cin >> c;
}

void sleep(int sec) {
	std::this_thread::sleep_for(std::chrono::seconds(sec));
}

void spam() {
	while (true) {
		std::cout << spamChar;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}


void threadFun() {

	//function_1();
	std::thread t1(function_1);	//t1 starts running
	//std::thread spamThread(spam);
	//spamThread.detach();
	std::cout << "This will come first probably\n";// << std::endl;

	std::thread t2(sleep, 1); //TODO: write down how to pass arguments in!!
	t2.join();
	std::cout << "Waited 1 sec" << std::endl;
	std::thread t4(sleep, 3);
	std::thread t3(sleep, 2);
	t3.join();
	std::cout << "Waited 2 secs " << std::endl;
	t4.join();
	std::cout << "Waited 1 sec more" << std::endl;



	//t1.detach();	//t1 will run freely on its own -- daemon process
	std::cout << "main!" << std::endl;
	std::cin >> spamChar;
	while (spamChar != 'a') {
		std::cin >> spamChar;
	}
	t1.join();	//main thread waits for t1 to finish HERE

	return;
}



void function_2() {
	std::cout << "hi" << std::endl;
}

void main() {

	LogFiles log("hi");
	std::thread t1(function_2);

	for (int i = 0; i < 100; i++) {
		std::cout << i << std::endl;
	}

	t1.join();
	char c;
	//std::cin >> c;
	TcpipServer server;
	server.runServer();
}


