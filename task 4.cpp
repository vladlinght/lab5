#include <winsock2.h>
#include <iostream>
#include <cstring>
#include <thread>

#pragma comment(lib, "ws2_32.lib")  // Підключення бібліотеки Winsock

#define SERVER_IP "127.0.0.1"  // IP сервера (локальний хост)
#define SERVER_PORT 12345      // Порт сервера
#define BUFFER_SIZE 512        // Розмір буфера для прийому/відправлення даних

// Функція для отримання повідомлень від сервера
void ReceiveMessages(SOCKET clientSocket, sockaddr_in& serverAddr) {
    char buffer[BUFFER_SIZE];
    int serverAddrSize = sizeof(serverAddr);
    int bytesReceived;

    while (true) {
        // Прийом повідомлень від сервера
        bytesReceived = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddr, &serverAddrSize);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed with error " << WSAGetLastError() << std::endl;
            continue;
        }

        buffer[bytesReceived] = '\0';  // Завершуємо рядок
        std::cout << "Received from server: " << buffer << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Ініціалізація Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Створення UDP-сокету
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Налаштування адреси сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Створюємо потік для отримання повідомлень від сервера
    std::thread receiveThread(ReceiveMessages, clientSocket, std::ref(serverAddr));

    std::cout << "Type your messages (type 'exit' to quit):\n";

    // Основний цикл клієнта для введення та відправки повідомлень
    while (true) {
        std::cin.getline(buffer, BUFFER_SIZE);

        // Вихід з чату
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Відправка повідомлення на сервер
        if (sendto(clientSocket, buffer, strlen(buffer), 0, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "sendto failed with error " << WSAGetLastError() << std::endl;
        }
    }

    // Закриття сокету
    closesocket(clientSocket);
    WSACleanup();

    // Завершення потоку
    receiveThread.join();

    return 0;
}
