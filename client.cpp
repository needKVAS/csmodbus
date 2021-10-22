#include <iostream>
#include <string>
#include <fstream>
#include "SocketHolder.h"
#include "CRC16.h"
#define BUFF_SIZE 2048
bool getIPoctet(std::string str, unsigned int &out);
bool getuint(std::string str, unsigned int &out);
int main()
{
	
	std::ifstream file;
	file.open ("client.txt", std::ifstream::in);
	if (!file.is_open())
	{
		std::cout << "Error opening file";
		return -1;
	}
	
	std::string buff;
	
	unsigned int ip1,ip2,ip3,ip4;
	unsigned int port;
	//
	//IP
	//////
	if(file.good())
	{
		std::getline(file,buff,'.');
	}
	else
	{
		std::cout << "Incorrect input file\n";
		return -2;
	}
	if(!getIPoctet(buff, ip1))
	{
		std::cout<<"Incorrect ip\n";
		return -3;
	}
	if(file.good())
	{
		std::getline(file,buff,'.');
	}
	else
	{
		std::cout << "Incorrect input file\n";
		return -2;
	}
	if(!getIPoctet(buff, ip2))
	{
		std::cout<<"Incorrect ip\n";
		return -3;
	}
	if(file.good())
	{
		std::getline(file,buff,'.');
	}
	else
	{
		std::cout << "Incorrect input file\n";
		return -2;
	}
	if(!getIPoctet(buff, ip3))
	{
		std::cout<<"Incorrect ip\n";
		return -3;
	}
	if(file.good())
	{
		std::getline(file,buff);
	}
	else
	{
		std::cout << "Incorrect input file\n";
		return -2;
	}
	if(!getIPoctet(buff, ip4))
	{
		std::cout<<"Incorrect ip\n";
		return -3;
	}
	//
	//Port
	/////////
	if(file.good())
	{
		std::getline(file,buff);
	}
	else
	{
		std::cout << "Incorrect input file\n";
		return -2;
	}
	if(!getuint(buff, port))
	{
		std::cout << "Incorrect port\n";
		return -3;
	}
	if((port>0xffff))
	{
		std::cout << "Incorrect port, port should not be more than 16 bit\n";
		return -3;
	}
	
	std::cout << "Host: "<< ip1 <<"."<< ip2 <<"."<< ip3 <<"." << ip4 <<"   "<< port <<"\n";
	
	SocketHolder::start();
	while(true)
	{
		unsigned int modbus_addr,first,num;
		int err;
		while(true)
		{
			std::cout <<"Modbus addres: ";
			std::getline(std::cin,buff);
			if(!getuint(buff, modbus_addr))
			{
				std::cout << "Incorrect modbus addres\n";
				continue;
			}
			if(modbus_addr>0xff)
			{
				std::cout <<"Incorrect modbus addres\n";
				std::cout <<"Modbus addres should not be more than 8 bit\n";
				continue;
			}
			else
			{
				break;
			}
		}
		while(true)
		{
			std::cout <<"First register: ";
			std::getline(std::cin,buff);
			if(!getuint(buff, first))
			{
				std::cout << "Incorrect first register\n";
				continue;
			}
			if(first>0xffff)
			{
				std::cout <<"First register should not be more than 16 bit\n";
				continue;
			}
			else
			{
				break;
			}
		}
		while(true)
		{
			std::cout <<"Number of registers: ";
			std::getline(std::cin,buff);
			if(!getuint(buff, num))
			{
				std::cout << "Incorrect number of registers\n";
				continue;
			}
			if(num>0xffff)
			{
				std::cout <<"Number of registers should not be more than 16 bit\n";
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
			std::cout <<"Server connect error\n";
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
			std::cout << "Send error (code: "<<client.getLastError()<<")\n";
			continue;
		}
		
		
		
		buff.clear();
		size_t length=0;
		do
		{
			recv_size = client.recv(buff_c, BUFF_SIZE);
			if(recv_size>0)
			{
				buff.reserve(recv_size);
				std::cout << "Received:\n";
				for(int k=0;k<recv_size; k++) 
				{
					printf("%x ", (unsigned char)buff_c[k]);
					buff.push_back(buff_c[k]);
				}
				std::cout << "\n";

			}
		}while(recv_size > 0);
		client.shutdown(0);
		if(buff.size()>0)
		{
			if(((unsigned char)buff[1])==0x3)
			{
				if(((unsigned char)buff[2])==(2*num))
				{
					std::cout <<"Register values: \n";
					for(int i=0; i<(2*num);i+=2) printf("%u: %x ",first+i/2,(((unsigned char)buff[3+i])<<8)|((unsigned char)buff[4+i]));
					std::cout <<"\n";
				}
				else
				{
					std::cout <<"Server returned incorrect values set\n";
				}
			}
			else
			{
				if(((unsigned char)buff[1])==0x83)
				{
					switch((unsigned char)buff[2])
					{
						case 0x1:
						std::cout <<"Server returned ILLEGAL FUNCTION  error\n";
						break;
						case 0x2:
						std::cout <<"Server returned ILLEGAL DATA ADDRESS  error\n";
						std::cout <<"Incorrect first register value\n";
						break;
						case 0x3:
						std::cout <<"Server returned ILLEGAL DATA VALUE  error\n";
						std::cout <<"Incorrect number of registers value\n";
						break;
					}
				}	
			}
		}
		else
		{
			std::cout <<"Server returned nothing\n";
			std::cout <<"Presumably the modbus address is incorrect\n";
		}
	}
	return 0;
}
bool getIPoctet(std::string str, unsigned int &out)
{
	size_t pos=str.find_first_not_of("0123456789 \t\r");
	if(pos!=std::string::npos)
	{
		return false;
	}
	try
	{
		out=stoi(str);
	}
	catch(std::invalid_argument& e)
	{
		return false;
	}
	catch(std::out_of_range& e)
	{
		return false;
	}
	if(out>0xff)
	{
		return false;
	}
	return true;
}
bool getuint(std::string str, unsigned int &out)
{
	size_t pos=str.find_first_not_of("0123456789 \t\r");
	if(pos!=std::string::npos)
	{
		return false;
	}
	try
	{
		out=stoi(str);
	}
	catch(std::invalid_argument& e)
	{
		return false;
	}
	catch(std::out_of_range& e)
	{
		return false;
	}
	return true;
}