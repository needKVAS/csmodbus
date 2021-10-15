# csmodbus
Compile on linux (g++ (Ubuntu 7.5.0-3ubuntu1~16.04) 7.5.0):  
```console
g++ client.cpp SocketHolder.cpp -std=c++11 -o client  
g++ server.cpp SocketHolder.cpp -std=c++11 -pthread -o server
```  
Compile on windows:  
MinGW (g++.exe (i686-posix-sjlj-rev0, Built by MinGW-W64 project) 8.1.0):  
```console
g++ client.cpp SocketHolder.cpp -lws2_32 -std=c++11 -o client  
g++ server.cpp SocketHolder.cpp -lws2_32 -std=c++11 -o server
 
