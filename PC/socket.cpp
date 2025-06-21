#include "main.h"

// подключение к серверу esp8266
uint8_t connectToServer(socket_t* _socket)
{
    // Подключение к серверу
    if (connect(_socket->clientSocket, (struct sockaddr*)&_socket->serverAddr, sizeof(_socket->serverAddr)) == SOCKET_ERROR) {
        printf("Connection failed: %d\n", WSAGetLastError()); closesocket(_socket->clientSocket); WSACleanup();
        return 1;
    }
    printf("Connected to server %s:%d\n", _socket->serverIP, _socket->serverPort);
    return 0;
}

int SetReceiveTimeout(SOCKET s, int timeout_ms) 
{
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
        (const char*)&timeout_ms, sizeof(timeout_ms)) == SOCKET_ERROR) {
        printf("setsockopt failed: %d\n", WSAGetLastError());
        return -1;
    }
    return 0;
}

// инициализируем сокет и подготавливаем его к подключению к серверу
uint8_t socketInit(socket_t* _socket)
{
    //Настройка и заполнение глобальной структуры
    unsigned char _filename[] = "C:\\Users\\Дмитрий\\Desktop\\FingerPrint\\prog\\fingerprint_103\\build\\debug\\fingerprint_103.bin";
    unsigned char _serverIP[] = "192.168.4.1";
    memcpy(_socket->serverIP, _serverIP, sizeof(_serverIP));
    _socket->serverPort = 8080;
    memset(_socket->sendBuffer, 0x00, BUFFER_SIZE+ADD_BITES);
    memset(_socket->recvBuffer, 0x00, BUFFER_SIZE);
    memcpy(_socket->filename, _filename, sizeof(_filename));

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &_socket->wsaData) != 0) {printf("WSAStartup failed: %d\n", WSAGetLastError()); return 1;}

    // Создание сокета
    _socket->clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket->clientSocket == INVALID_SOCKET) { printf("Socket creation failed: %d\n", WSAGetLastError()); WSACleanup(); return 1;}

    // Настройка адреса сервера
    _socket->serverAddr.sin_family = AF_INET;
    _socket->serverAddr.sin_addr.s_addr = inet_addr(_socket->serverIP);
    _socket->serverAddr.sin_port = htons(_socket->serverPort);

    SetReceiveTimeout(_socket->clientSocket, RECIVE_TIMEOUT);

    return 0;
}


/************************************************/
/*			Передача файла на сервер			*/
/************************************************/

uint8_t readFile(socket_t* _socket)
{
    FILE* file = NULL;
    size_t bytes_read;
    size_t total_bytes = 0;
    uint16_t packetNum = 0;
    uint8_t TXAgain = 0;

    // Безопасное открытие файла
    errno_t err = fopen_s(&file, _socket->filename, "rb");
    if (err != 0 || file == NULL) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    // выводим информацию о файле и очищаем входные и выходные буфферы
    printf("'%s' to %s:%d\n", _socket->filename, _socket->serverIP, _socket->serverPort);
    memset(_socket->sendBuffer, 0x00, BUFFER_SIZE + ADD_BITES);
    memset(_socket->recvBuffer, 0x00, BUFFER_SIZE);

    while ((bytes_read = fread(_socket->sendBuffer, 1, CHUNK_SIZE, file)) > 0) {

        // Выводим информацию о считанных байтах, считаем контрольную сумму и выводим первые 32 считанных байта
        total_bytes += bytes_read;
        printf("[%zu]\t", bytes_read);
        uint16_t sum = 0;
        for (size_t i = 0; i < bytes_read; i++) {
            if (i < 32) printf("%02X ", _socket->sendBuffer[i]);
            sum += _socket->sendBuffer[i];
        }
        memcpy(&_socket->sendBuffer[bytes_read], &packetNum, 2);
        sum += (_socket->sendBuffer[bytes_read] + _socket->sendBuffer[bytes_read + 1]);
        // копируем контрольную сумму в конец пакета
        memcpy(&_socket->sendBuffer[bytes_read + 2], &sum, 2);
        printf("\t[0x%x]\r\n", sum);

        // пытаемся передать пакет
        while (1) {
            // отсылаем пакет
            int sendRes = send(_socket->clientSocket, (const char*)_socket->sendBuffer, bytes_read + ADD_BITES, 0);
            if (sendRes == SOCKET_ERROR) {
                printf("Send failed: %d\n", WSAGetLastError());
                return EXIT_FAILURE;
            }

            // ждём подтверждения. Если пакет доставлен и контрольная сумма сходится, то сервер ответит 'O', иначе ответит 'B'
            int recvRes = recv(_socket->clientSocket, _socket->recvBuffer, BUFFER_SIZE, 0);
            if (recvRes != SOCKET_ERROR) {
                _socket->recvBuffer[recvRes] = '\0';
                Sleep(5);
                if (strstr(_socket->recvBuffer, "O")) {
                    TXAgain = 0;
                    packetNum++;
                    break;
                }
                else if (strstr(_socket->recvBuffer, "B")) {
                    TXAgain++;
                    printf("sum is bad. transmit again\n");
                }
            }
            else {
                TXAgain++;
                printf("server time out\r\n");
            }
            if (TXAgain >= TX_AGAIN) {
                printf("TX again too mutch\r\n");
                return EXIT_FAILURE;
            }
        }
    }
    send(_socket->clientSocket, (char*)"finish_transmit", 16, 0);
    Sleep(200);
    if (ferror(file)) {
        perror("Ошибка чтения файла");
        fclose(file);
        return EXIT_FAILURE;
    }

    fclose(file);
    printf("done: %zu byte.\n", total_bytes);
    return EXIT_SUCCESS;
}