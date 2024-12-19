#include <iostream>
#include <fstream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 1024

void receiveFile(SOCKET serverSocket, sockaddr_in& clientAddr, int clientAddrSize) {
    char buffer[BUFFER_SIZE];

    // Получаем имя файла
    int len = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
    buffer[len] = '\0';
    std::string filename = std::string(buffer);
    std::cout << "Receive the file: " << filename << std::endl;

    // Отправляем подтверждение (ACK) на имя файла
    sendto(serverSocket, "ACK", 3, 0, (sockaddr*)&clientAddr, clientAddrSize);

    // Получаем размер файла
    uint32_t fileSize;
    recvfrom(serverSocket, (char*)&fileSize, sizeof(fileSize), 0, (sockaddr*)&clientAddr, &clientAddrSize);
    std::cout << "File size: " << fileSize << " bytes\n";

    // Отправляем подтверждение (ACK) на размер файла
    sendto(serverSocket, "ACK", 3, 0, (sockaddr*)&clientAddr, clientAddrSize);

    // Получаем содержимое файла
    std::ofstream outFile(filename, std::ios::binary);
    uint32_t receivedBytes = 0;
    while (receivedBytes < fileSize) {
        int len = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
        outFile.write(buffer, len);
        receivedBytes += len;
    }
    outFile.close();
    std::cout << "File saved.\n";
}

void sendFile(SOCKET serverSocket, sockaddr_in& clientAddr, int clientAddrSize) {
    char buffer[BUFFER_SIZE];

    // Получаем запрос имени файла
    int len = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
    buffer[len] = '\0';
    std::string filename = buffer;
    std::cout << "Request file: " << filename << std::endl;

    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "file not found.\n";
        return;
    }

    // Отправляем размер файла
    inFile.seekg(0, std::ios::end);
    uint32_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    sendto(serverSocket, (char*)&fileSize, sizeof(fileSize), 0, (sockaddr*)&clientAddr, clientAddrSize);

    // Получаем подтверждение (ACK)
    recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);

    // Отправляем содержимое файла
    while (!inFile.eof()) {
        inFile.read(buffer, BUFFER_SIZE);
        int bytesRead = inFile.gcount();
        sendto(serverSocket, buffer, bytesRead, 0, (sockaddr*)&clientAddr, clientAddrSize);
    }
    inFile.close();
    std::cout << "File sent!\n";
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr, clientAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "UDP-server started. Waiting for client...\n";

    int clientAddrSize = sizeof(clientAddr);
    char command[BUFFER_SIZE];

    // Основной цикл
    while (true) {
        int len = recvfrom(serverSocket, command, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
        command[len] = '\0';

        if (std::string(command) == "UPLOAD") {
            receiveFile(serverSocket, clientAddr, clientAddrSize);
        }
        else if (std::string(command) == "DOWNLOAD") {
            sendFile(serverSocket, clientAddr, clientAddrSize);
        }
        else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}