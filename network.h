#ifndef _NETWORK_H_
#define _NETWORK_H_
#define SERVER_IP "127.0.0.1" 


void endServerHandler(int sig);

int connectToServer(char* serverIP, int serverPort);

#endif