
#include "SocketHolder.h"
#include <iostream>
#ifdef _WIN32
WSAData SocketHolder::ws=WSAData(); 
#endif
SocketHolder::SocketHolder(int Family, const char* addr, u_short port)
: err(0), family(Family)
{
	socket_addr.sin_family = family;
	#ifdef _WIN32
	InetPtonA(family,addr, &socket_addr.sin_addr.s_addr);
	#else
	socket_addr.sin_addr.s_addr=inet_addr(addr);
	#endif
	socket_addr.sin_port = htons(port);
	hsocket = socket(Family, SOCK_STREAM, 0);
}
#ifdef _WIN32
SocketHolder::SocketHolder(int Family, sockaddr_in addr, SOCKET sock)
: err(0), family(Family),hsocket(sock),socket_addr(addr)
{}
#else
SocketHolder::SocketHolder(int Family, sockaddr_in addr, int sock)
: err(0), family(Family),hsocket(sock),socket_addr(addr)
{}
#endif
int SocketHolder::connect()
{
	return ::connect(hsocket, (sockaddr*)&socket_addr, sizeof(socket_addr));
}
int SocketHolder::listen(int backlog)
{
	return ::listen(hsocket,backlog);
}
void SocketHolder::accept(std::list<SocketHolder>* outlist)
{
	sockaddr_in addr;
	socklen_t addrlen=socklen_t(sizeof(addr));;
	#ifdef _WIN32
	SOCKET out;
	#else
	int out;
	#endif
	out=::accept(hsocket,(sockaddr*)&addr,&addrlen);
	if (out == -1)
	{
		std::cerr << "accept err\n";
		return;
	}
	outlist->emplace_back(family,addr,out);
}
int SocketHolder::bind()
{
	return ::bind(hsocket, (sockaddr*)&socket_addr, sizeof(socket_addr));
}
SocketHolder::~SocketHolder()
{	
	#ifdef _WIN32
	closesocket(hsocket);
	#else
	close(hsocket);
	#endif
}
int SocketHolder::start()
{
	#ifdef _WIN32
	return WSAStartup(MAKEWORD(1, 1), &ws);
	#else
	return 0;
	#endif
}
int SocketHolder::send(const char* buffer, const size_t size)
{
	return ::send(hsocket, buffer, size, 0);
}
int SocketHolder::recv(char *buf, int len)
{
	return ::recv(hsocket, buf, len, 0);
}
int SocketHolder::shutdown(int how)
{
	return ::shutdown(hsocket,how);
}
bool SocketHolder::is_invalid_socket()
{
	if (hsocket==-1)	return true;
	return false;
}
int SocketHolder::getLastError()
{
	#ifdef _WIN32
	return WSAGetLastError();
	#else
	return errno;
	#endif
}

