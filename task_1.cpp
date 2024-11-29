#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")  // Підключаємо бібліотеку Winsock

#define PORT 12345  // Порт для прослуховування
#define BUFFER_SIZE 512  // Розмір буфера для обміну даними

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    // Ініціалізація Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Створення серверного сокету
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Налаштування адреси сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Слухати на всіх інтерфейсах
    serverAddr.sin_port = htons(PORT);

    // Прив'язка сокету до порту
    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Прослуховування з'єднань
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    // Прийом з'єднання від клієнта
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected from " << inet_ntoa(clientAddr.sin_addr) << std::endl;

    // Отримання та обробка запитів від клієнта
    while (true) {
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Recv failed with error " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0';  // Завершення рядка

        std::cout << "Received message: " << buffer << std::endl;

        // Відправка відповіді клієнту
        const char* response = "Message received!";
        if (send(clientSocket, response, strlen(response), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed with error " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // Закриття з'єднання з клієнтом та сокету сервера
    closesocket(clientSocket);
    closesocket(serverSocket);

    // Завершення Winsock
    WSACleanup();

    return 0;
}
