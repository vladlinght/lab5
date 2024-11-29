#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")  // Підключення бібліотеки Winsock

#define SERVER_PORT 12345  // Порт сервера
#define BUFFER_SIZE 512  // Розмір буфера для отримання та відправки даних

// Функція для обробки запитів клієнтів
DWORD WINAPI ClientHandler(LPVOID lpParam) {
    SOCKET clientSocket = (SOCKET)lpParam;  // Отримуємо сокет клієнта
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    // Виведення повідомлення про нове з'єднання
    std::cout << "Client connected. Handling client..." << std::endl;

    // Отримання даних від клієнта
    bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Recv failed with error " << WSAGetLastError() << std::endl;
    } else if (bytesReceived == 0) {
        std::cout << "Client closed connection." << std::endl;
    } else {
        buffer[bytesReceived] = '\0';  // Завершуємо рядок
        std::cout << "Received from client: " << buffer << std::endl;

        // Відправка відповіді клієнту
        const char* response = "Hello from server!";
        send(clientSocket, response, strlen(response), 0);
    }

    // Закриття з'єднання з клієнтом
    closesocket(clientSocket);

    std::cout << "Client connection closed." << std::endl;

    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);

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
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Приймає з'єднання з будь-якої адреси
    serverAddr.sin_port = htons(SERVER_PORT);

    // Прив'язка сокету до порту
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Слухання порту
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port " << SERVER_PORT << "..." << std::endl;

    // Основний цикл сервера для прийому нових з'єднань
    while (true) {
        // Прийом клієнтського з'єднання
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error " << WSAGetLastError() << std::endl;
            continue;  // Якщо не вдалося прийняти з'єднання, продовжуємо прийом наступного клієнта
        }

        // Створення нового потоку для обробки клієнта
        HANDLE hThread = CreateThread(NULL, 0, ClientHandler, (LPVOID)clientSocket, 0, NULL);
        if (hThread == NULL) {
            std::cerr << "CreateThread failed with error " << GetLastError() << std::endl;
            closesocket(clientSocket);
        } else {
            CloseHandle(hThread);  // Закриваємо дескриптор потоку, він буде виконуватися асинхронно
        }
    }

    // Закриття серверного сокету
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
