#ifndef SOCKETHOLDER_H
#define SOCKETHOLDER_H


#ifdef _WIN32 
	#ifndef WINVER
	#define WINVER 0x0601
	#endif
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0601
	#endif
	#include <WinSock2.h>
	#include <Ws2def.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <errno.h>
#endif
#include<list>

class SocketHolder
{
	private:
	#ifdef _WIN32 
	static WSAData ws;
	SOCKET hsocket;
	#else
	int hsocket;
	#endif
	sockaddr_in socket_addr;
	int err;
	int family;
	SocketHolder(const SocketHolder&) = delete;
	void operator=(const SocketHolder&) = delete;
	public:
	#ifdef _WIN32 
	SocketHolder(int Family, sockaddr_in addr, SOCKET sock);
	#else
	SocketHolder(int Family, sockaddr_in addr, int sock);
	#endif
	SocketHolder(int Family, const char* addr, u_short port);
	~SocketHolder();
	static int start();
	void accept(std::list<SocketHolder>*);
	int listen(int backlog);
	int connect();
	int connect(int timeout);
	int bind();
	int send(const char* buffer, const size_t size);
	int recv(char *buf, int len);
	int shutdown(int how);
	int getLastError();
	bool is_invalid_socket();
};

#endif // SOCKETHOLDER_H
