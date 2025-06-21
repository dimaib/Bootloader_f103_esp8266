#include "main.h"

#include <iostream>
#include <string>

socket_t socketARM;

#define SIZE_MENU_ITEM      100
char menu[][SIZE_MENU_ITEM] = {
    "[1] Прошивка через wifi\r\n",
    "[2] Конфигурация wifi модуля\r\n"
};

int main() 
{
    SetConsoleOutputCP(1251);
    //char getStr[100];
    //printf("Меню:\r\n");
    //for (uint8_t i = 0; i < sizeof(menu) / SIZE_MENU_ITEM; i++) printf("%s", menu[i]);
    //while (1) {
    //    printf("Номер пункта меню:");
    //    if (fgets(getStr, 2, stdin) == NULL) {
    //        printf("Ошибка ввода!\n");
    //        return 1;
    //    }
    //    if (getStr[0] < '0' || getStr[0]>'9') {
    //        printf("не число!\r\n");
    //    }
    //    printf("%s\r\n", getStr);
    //    //getStr[0] = -52; getStr[1] = -52;

    //}
    //return 0;
    //инициализация и запуск клиента
    if (socketInit(&socketARM)) return 1;
    if (connectToServer(&socketARM)) return 1;

    printf("press enter for start..\r\n");
    getchar();
    
    int sendRes = send(socketARM.clientSocket, "start_transmit", 15, 0);
    if (sendRes == SOCKET_ERROR) {
        printf("Send failed: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    int recvRes = recv(socketARM.clientSocket, socketARM.recvBuffer, BUFFER_SIZE, 0);
    if (recvRes != SOCKET_ERROR) {
        if (strstr(socketARM.recvBuffer, "R")) {
            printf("server ready\r\n");
            Sleep(200);
        }
    } else {
        printf("server no ack. Exit..\r\n");
        return 0;
    }

    getchar();

    if (readFile(&socketARM)) {
        printf("file trancmited fail\r\n");
    }else{
        printf("file trancmited success\r\n");
    }

    // Завершение работы
    closesocket(socketARM.clientSocket);
    WSACleanup();
    printf("Disconnected\n");
    return 0;
}