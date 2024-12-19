#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>  // Для InetPtonA
#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define BUFFER_SIZE 1024

void sendFileToServer(SOCKET clientSocket, sockaddr_in& serverAddr) {
    char buffer[BUFFER_SIZE];

    // Ввод имени файла
    std::cout << "Enter the file name to send: ";
    std::string filename;
    std::cin >> filename;

    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: Failed to open file.\n";
        return;
    }

    // Отправляем команду "UPLOAD"
    std::string command = "UPLOAD";
    sendto(clientSocket, command.c_str(), command.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Отправляем имя файла
    sendto(clientSocket, filename.c_str(), filename.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Получаем подтверждение (ACK)
    recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);

    // Отправляем размер файла
    inFile.seekg(0, std::ios::end);
    uint32_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    sendto(clientSocket, (char*)&fileSize, sizeof(fileSize), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Получаем подтверждение (ACK)
    recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);

    // Отправляем файл блоками
    while (!inFile.eof()) {
        inFile.read(buffer, BUFFER_SIZE);
        int bytesRead = inFile.gcount();
        sendto(clientSocket, buffer, bytesRead, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    }
    inFile.close();
    std::cout << "The file was successfully sent to the server.\n";
}

void receiveFileFromServer(SOCKET clientSocket, sockaddr_in& serverAddr) {
    char buffer[BUFFER_SIZE];

    // Отправляем команду "DOWNLOAD"
    std::string command = "DOWNLOAD";
    sendto(clientSocket, command.c_str(), command.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Вводим имя файла для скачивания
    std::cout << "Enter the name of the file to download: ";
    std::string filename;
    std::cin >> filename;

    sendto(clientSocket, filename.c_str(), filename.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Получаем размер файла
    uint32_t fileSize;
    recvfrom(clientSocket, (char*)&fileSize, sizeof(fileSize), 0, NULL, NULL);

    // Отправляем подтверждение (ACK)
    sendto(clientSocket, "ACK", 3, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Получаем файл
    std::ofstream outFile(filename, std::ios::binary);
    uint32_t receivedBytes = 0;
    while (receivedBytes < fileSize) {
        int len = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        outFile.write(buffer, len);
        receivedBytes += len;
    }
    outFile.close();
    std::cout << "The file was successfully received from the server.\n";
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    InetPtonA(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    while (true) {
        std::cout << "\n1. Send file to server\n";
        std::cout << "2. Download file to server\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose option: ";
        int choice;
        std::cin >> choice;

        if (choice == 1) {
            sendFileToServer(clientSocket, serverAddr);
        }
        else if (choice == 2) {
            receiveFileFromServer(clientSocket, serverAddr);
        }
        else if (choice == 3) {
            break;
        }
        else {
            std::cout << "Wrong choice. Try again.\n";
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}