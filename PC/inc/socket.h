#pragma once
#ifndef SOCKET_H
#define SOCKET_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#define RECIVE_TIMEOUT      2000
#define TX_AGAIN            10

#define ADD_BITES           4
#define BUFFER_SIZE			1024
#define CHUNK_SIZE			1024

typedef struct {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char serverIP[20];// = "192.168.4.1";
    int serverPort;// = 8080;
    unsigned char sendBuffer[BUFFER_SIZE+ADD_BITES];// = "dimaib";
    char recvBuffer[BUFFER_SIZE];
    char filename[BUFFER_SIZE*2];
}socket_t;

uint8_t socketInit(socket_t* _socket);
uint8_t connectToServer(socket_t* _socket);
uint8_t readFile(socket_t* _socket);

#endif	//SOCKET_H