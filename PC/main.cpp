#include "main.h"

#include <iostream>
#include <string>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "io.h"

socket_t socketARM;

//#define ESP_BUF_SIZE        1200

#define STR_MAX_SIZE_IN_FLASH   128

typedef struct {
    uint8_t server[STR_MAX_SIZE_IN_FLASH];
    uint8_t port[STR_MAX_SIZE_IN_FLASH];
    uint8_t apName[STR_MAX_SIZE_IN_FLASH];
    uint8_t apPass[STR_MAX_SIZE_IN_FLASH];
}param_t;

uint8_t searchChar(uint8_t* buff, uint8_t _char, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
        if (buff[i] == _char) return i;
    return 0;
}

uint8_t getParamFromLine(uint8_t* param, char* line)
{
    uint8_t posBegin = searchChar((uint8_t *)line, '"', strlen(line));
    uint8_t posEnd = searchChar((uint8_t*)&line[posBegin+1], '"', strlen(line));
    if (posBegin && posEnd) {
        memcpy(param, &line[posBegin+1], posEnd);
        //printf("%s\r\n", param);
        return 0;
    }
    return 1;
}

uint8_t readSettings(char* _file, param_t *param)
{
    FILE* file;
    errno_t err;
    char line[256]; // Буфер для одной строки

    err = fopen_s(&file, _file, "r");
    if (err != 0 || file == NULL) {
        perror("Error opening file");
        return 1;
    }
    printf("\r\n\r\nSettings from '%s':\r\n", _file);
    while (fgets(line, sizeof(line), file)) {
        //printf("%s", line);
        if (strstr(line, "server_address")) {
            getParamFromLine(param->server, line);
            printf(" -server address|  %s\r\n", param->server);
        } else if (strstr(line, "server_port")) {
            getParamFromLine(param->port, line);
            printf(" -server port\t|  %s\r\n", param->port);
        } else if (strstr(line, "ap_name")) {
            getParamFromLine(param->apName, line);
            printf(" -ap name \t|  %s\r\n", param->apName);
        } else if (strstr(line, "ap_pass")) {
            getParamFromLine(param->apPass, line);
            printf(" -ap password\t|  %s\r\n", param->apPass);
        }
    }

    fclose(file);
    return 0;
}

#define MAX_PATH        2048

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(1251);
    param_t parameters = {"", "", "", ""};
    //инициализация и запуск клиента

    uint8_t settingsMode = 0;
    wchar_t  path[MAX_PATH];
    char  _file[MAX_PATH]="";

    if (argc < 2) {
        printf("Запуск программы предусмотрен только с использованием параметров коммандной строки.\r\nПеретащите нужные файлы [*.cfg/*.bin] на исполняемый файл..");
        getchar();
        return 1;
    }
    if (_access(argv[1], 0)) {
        printf("Файл \"%s\" не существует или нет доступа.\n", argv[1]);
        getchar();
        return 1;
    }
    memcpy(_file, argv[1], strlen(argv[1]));
    if (strstr(&_file[strlen(_file) - 4], ".cfg")) { printf("File of settings\r\n"); settingsMode = 1; }


    if (settingsMode) {
        
        readSettings(_file, &parameters);
        printf("Передать новые настройки в устройство?\r\n[Enter - для продолжения или закрыть программу для отмены]\r\n");
        getchar();
        printf("Подключение к устройству..\r\n");
        if (socketInit(&socketARM)) return 1;
        if (connectToServer(&socketARM)) return 1;
        printf("Успешно..\r\n");
        // подготовка строки к отправке и отправка пакета на изменение параметра
        char commandsArr[][STR_MAX_SIZE_IN_FLASH] = {"setServer=", "setPort=", "setAPN=", "setAPP="};   // массив с командами
        char* p = (char*) &parameters.server[0];                                                        // метка для удобного перемещения по структуре параметров
        for (uint8_t i = 0; i < 4; i++) {
            char tmpBuff[255] = "";                                                                     // временный массив для построения строки
            memcpy(tmpBuff, commandsArr[i], strlen(commandsArr[i]));                                    // копируем комманду
            strcat_s(tmpBuff, 255, p);                                                                  // объединяем две строки команда + параметр
            send(socketARM.clientSocket, tmpBuff, strlen(tmpBuff), 0);                                  // посылаем строку на сервер
            Sleep(200);                                                                                 // ждём, пока сервер обработает посылку
            p += STR_MAX_SIZE_IN_FLASH;                                                                 // перемещаемся к следующему параметру в структуре
        }                                                                                               // это работает, если все поля структуры равны по размеру.
        printf("Данные отправлены..\r\n[Enter - для выхода]");
        getchar();
        return 0;
    }                                                                                                   // в данном случае все поля это массивы размером STR_MAX_SIZE_IN_FLASH
    return 0;

    if (socketInit(&socketARM)) return 1;
    if (connectToServer(&socketARM)) return 1;
    // начало передачи файла
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