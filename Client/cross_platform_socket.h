#pragma once
#ifndef CROSS_PLATFORM_SOCKETS
#define CROSS_PLATFORM_SOCKETS

using namespace std;


#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define close_socket closesocket
#define socklen_t int
#define SHUT_RDWR SD_BOTH
#define MSG_NOSIGNAL 0

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define close_socket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>


inline void init_network() {
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		runtime_error("Ошибка инициализации Winsock");
	}
#endif
}


inline void cleanup_network() {
#ifdef _WIN32
	WSACleanup();
#endif
}


inline void check_socket_error(int result, const string& error_msg) {

	if (result == SOCKET_ERROR) {
#ifdef _WIN32
		int error = WSAGetLastError();
		throw runtime_error(error_msg + ". Code error: " + to_string(error));

#else
		perror(error_msg.c_str());
		throw runtime_error(error_msg);
#endif
	}
}

#endif  //CROSS_PLATFORM_SOCKETS