#include <iostream>
#include <string>
#include "SocketHolder.h"
#include "CRC16.h"
#define BUFF_SIZE 2048
int main()
{
	FILE * pif;
	pif = fopen ("client.txt","r");
	unsigned int ip1,ip2,ip3,ip4;
	unsigned int port;
	int result=fscanf(pif, "%u.%u.%u.%u\n%u", &ip1,&ip2,&ip3,&ip4,&port);
	if(result!=5)
	{
		printf("Incorrect input file\n");
		return -1;
	}
	if((ip1>255)&&(ip2>255)&&(ip3>255)&&(ip4>255))
	{
		printf("Incorrect ip\n");
		return -2;
	}
	//printf("Result: %d\n",result);
	//printf("Readed: %u.%u.%u.%u\n%u\n",ip1,ip2,ip3,ip4,port);
	
	SocketHolder::start();
	while(true)
	{
		unsigned int modbus_addr,first,num;
		int err;
		while(true)
		{
			printf("\nModbus addres: ");
			fflush(stdin);
			err=scanf("%u",&modbus_addr);
			if((err==-1)||(err==0))
			{
				printf("\nIncorrect modbus addres");
				continue;
			}
			//printf("Scanned modbus addres: %u",modbus_addr);
			if(modbus_addr>0xff)
			{
				printf("\nIncorrect modbus addres");
				printf("\nModbus addres should not be more than 8 bit");
				continue;
			}
			else
			{
				break;
			}
		}
		while(true)
		{
			printf("\nFirst register: ");
			fflush(stdin);
			err=scanf("%u",&first);
			if((err==-1)||(err==0))
			{
				printf("\nIncorrect first register");
				continue;
			}
			//printf("Scanned modbus addres: %u",modbus_addr);
			if(first>0xffff)
			{
				printf("\nFirst register should not be more than 16 bit");
				continue;
			}
			else
			{
				break;
			}
		}
		while(true)
		{
			printf("\nNumber of registers: ");
			fflush(stdin);
			err=scanf("%u",&num);
			if((err==-1)||(err==0))
			{
				printf("\nIncorrect number of registers");
				continue;
			}
			//printf("Scanned modbus addres: %u",modbus_addr);
			if(first>0xffff)
			{
				printf("\nNumber of registers should not be more than 16 bit");
				continue;
			}
			else
			{
				break;
			}
		}

		
		SocketHolder client(AF_INET, std::string(std::to_string(ip1)+'.'+std::to_string(ip2)+'.'+std::to_string(ip3)+'.'+std::to_string(ip4)).c_str(),port);
		std::string buff; 
		char buff_c[BUFF_SIZE+1];
		int recv_size;
		recv_size=client.connect();
		if(recv_size==-1) 
		{
		printf("Server connect error\n");
			return -3;
		}
		
		buff.reserve(8);
		buff.push_back(modbus_addr);
		buff.push_back(3);
		buff.push_back((first>>8)&0xff);
		buff.push_back((first)&0xff);
		buff.push_back((num>>8)&0xff);
		buff.push_back((num)&0xff);
		uint16_t control=CRC16((uint8_t*)&buff[0], buff.size());
		buff.push_back((control>>8)&0xff);
		buff.push_back((control)&0xff);
		recv_size=client.send(&buff[0], buff.size());
			
		client.shutdown(1);
		
		if(recv_size==-1) 
		{
			printf("Send error (code: %d)\n", client.getLastError());
			continue;
		}
		
		
		
		buff.clear();
		size_t length=0;
		do
		{
			recv_size = client.recv(buff_c, BUFF_SIZE);
			buff.reserve(recv_size);
			if(recv_size>0)
			{
				printf("\nReceived:\n");
				for(int k=0;k<recv_size; k++) 
				{
					printf("%x ", (unsigned char)buff_c[k]);
					buff.push_back(buff_c[k]);
				}
			}
		}while(recv_size > 0);
		printf("\n");
		client.shutdown(0);
		
		if(((unsigned char)buff[1])==0x3)
		{
			if(((unsigned char)buff[2])==(2*num))
			{
				printf("\nRegister values: \n");
				for(int i=0; i<(2*num);i+=2) printf("%u: %x ",first+i/2,(((unsigned char)buff[3+i])<<8)|((unsigned char)buff[4+i]));
				printf("\n");
			}
			else
			{
				printf("Server returned incorrect values set\n");
			}
		}
		else
		{
			if(((unsigned char)buff[1])==0x83)
			{
				switch((unsigned char)buff[2])
				{
					case 0x1:
					printf("Server returned ILLEGAL FUNCTION  error\n");
					break;
					case 0x2:
					printf("Server returned ILLEGAL DATA ADDRESS  error\n");
					printf("Incorrect first register value\n");
					break;
					case 0x3:
					printf("Server returned ILLEGAL DATA VALUE  error\n");
					printf("Incorrect number of registers value\n");
					break;
				}
			}	
		}
	}
	return 0;
}
