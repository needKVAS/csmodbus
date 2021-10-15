#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <mutex>
#include "SocketHolder.h"
#include "CRC16.h"
#define BUFF_SIZE 2048
#define MAX_CLIENTS 100
void recvandsend(SocketHolder *sock_clt, int*inactive, int modbus_addr );
std::mutex mtx;
std::string error_buff(const std::string& buff, int code);

uint16_t data[10]={1,2,3,4,5,6,7,8,9,10};

int main()
{
	FILE * pif;
	pif = fopen ("server.txt","r");
	unsigned int modbus_addr;
	int port;
	int result=fscanf(pif, "%u\n%u\n", &modbus_addr,&port);
	/*printf("Result: %d\n",result);
	printf("Readed: %u\n%u\n",modbus_addr,port);*/
	if(result!=2)
	{
		printf("Incorrect input file\n");
		return -1;
	}
	if((0==modbus_addr)&&(modbus_addr>247))
	{
		printf("Incorrect modbus address\n");
		return -2;
	}
	
	
	for(int i=0;i<10;i++) data[i]=i+1;
	
	SocketHolder::start();
	SocketHolder server(AF_INET,"127.0.0.1",port);
	if(server.bind()==-1) 
	{
		printf("Bind error (code: %d)\n", server.getLastError());
		return -3;
	}
	if(server.listen(5)==-1) 
	{
		printf("Listen error (code: %d)\n", server.getLastError());
		return -4;
	}
	
	std::list<SocketHolder> clients;
	std::vector<std::list<SocketHolder>::iterator> clients_iter(100, clients.end());
	std::vector<int> inactive_clients(100,2); 
	
	while(true)
	{
		int j=-1;
		mtx.lock();
		for (int i = 0; i<inactive_clients.size(); i++)
		{
			if(inactive_clients[i]==2) 
			{
				inactive_clients[i] = 0;
				j=i;
				break;
			}
		}
		mtx.unlock();
		if (j != -1)
		{
			server.accept(&clients);
			clients_iter[j] = --clients.end();
			std::thread(recvandsend, &(*clients_iter[j]), &(inactive_clients[j]), modbus_addr).detach();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		mtx.lock();
		for(int i=0;i<inactive_clients.size();i++) 
		{
			if(inactive_clients[i]==1) 
			{
				clients.erase(clients_iter[i]);
				inactive_clients[i]=2;
			}
		}
		mtx.unlock();
	}
    return 0;
}
std::string error_buff(const std::string& buff, int code)
{
	std::string out;
	out.reserve(5);
	out.push_back(buff[0]);
	out.push_back(buff[1]|0x80);
	out.push_back((code)&0xff);
	uint16_t control=CRC16((uint8_t*)&out[0], out.size());
	out.push_back((control>>8)&0xff);
	out.push_back((control)&0xff);
	return out;
}

void recvandsend(SocketHolder *sock_clt, int*inactive, int modbus_addr)
{
	std::string buff; 
	int recv_size;
	char buff_c[BUFF_SIZE];
	do
	{
		recv_size = sock_clt->recv(buff_c, BUFF_SIZE);
		if(recv_size==-1) printf("Recv error (code: %d)\n", sock_clt->getLastError());
		
		if (recv_size > 0)
		{
			buff.reserve(recv_size);
			printf("Received:\n");
			for(int k=0;k<recv_size; k++) 
			{
				printf("%x ", (unsigned char)buff_c[k]);
				buff.push_back(buff_c[k]);
			}
			printf("\n");
		}
	}while(recv_size > 0);
	sock_clt->shutdown(0);
	
	
	unsigned int first,num;
	
	if((buff[0]==modbus_addr)||(buff[0]==0))
	{
		if(buff[1]==0x3)
		{
			first=(buff[2]<<8)|(buff[3]);
			if((first<=10)&&(first>0))
			{
				num=(buff[4]<<8)|(buff[5]);
				if((num>0)&&((first+num-1)<=10))
				{
					buff.clear();
					buff.reserve(5+2*num);
					buff.push_back(modbus_addr);
					buff.push_back(3);
					buff.push_back(2*num);
					for(int k=0; k<num; k++)
					{
						buff.push_back((data[first-1+k]>>8)&0xff);
						buff.push_back((data[first-1+k])&0xff);
					}
					uint16_t control=CRC16((uint8_t*)&buff[0], buff.size());
					buff.push_back((control>>8)&0xff);
					buff.push_back((control)&0xff);
				}
				else
				{
					buff=error_buff(buff,0x3);
				}
			}
			else
			{
				buff=error_buff(buff,0x2);
			}
		}
		else
		{
			buff=error_buff(buff,0x1);
		}
		
		
		recv_size=sock_clt->send(&buff[0], buff.size());
		sock_clt->shutdown(1);
	}
	
	mtx.lock();
	*inactive=1;
	mtx.unlock();

	return;
}