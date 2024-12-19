#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include<iostream>
#include<fstream>
#include<string>
#include<experimental/filesystem>
#pragma warning(disable:4996)
#pragma comment (lib, "ws2_32.lib")
#include<WinSock2.h>

using namespace std;

bool file_ready_for_confirmation = false; // Flag indicating the readiness of the file for confirmation

void send_file(SOCKET* sock) {
    string file_name;
    cout << "Enter the file name to send to the client: ";
    cin >> file_name;

    fstream file;
    file.open(file_name, ios_base::in | ios_base::binary);

    if (file.is_open()) {
        int file_size = experimental::filesystem::file_size(file_name);
        char* bytes = new char[file_size];
        file.read(bytes, file_size);

        send(*sock, to_string(file_size).c_str(), 16, 0);
        send(*sock, file_name.c_str(), 32, 0);
        send(*sock, bytes, file_size, 0);

        cout << "File \"" << file_name << "\" successfully sent to the client.\n";
        file_ready_for_confirmation = true;

        delete[] bytes;
    }
    else {
        cout << "Error: File not found.\n";
    }

    file.close();
}

void recv_file(SOCKET* sock) {
    char file_size_str[16];
    char file_name[32];

    recv(*sock, file_size_str, 16, 0);
    int file_size = atoi(file_size_str);
    char* bytes = new char[file_size];

    recv(*sock, file_name, 32, 0);

    fstream file;
    file.open(file_name, ios_base::out | ios_base::binary);

    if (file.is_open()) {
        recv(*sock, bytes, file_size, 0);
        file.write(bytes, file_size);
        cout << "File \"" << file_name << "\" successfully received from the client.\n";
        file_ready_for_confirmation = true;
    }
    else {
        cout << "Error saving the file.\n";
    }

    delete[] bytes;
    file.close();
}


int main() {
    cout << "=== SERVER ===" << endl;

    WORD dllVer = MAKEWORD(2, 1);
    WSAData wsad;
    WSAStartup(dllVer, &wsad);

    SOCKADDR_IN addr_info;
    memset(&addr_info, 0, sizeof(SOCKADDR_IN));

    int size_addr = sizeof(addr_info);
    addr_info.sin_port = htons(4321);
    addr_info.sin_family = AF_INET;

    SOCKET s_listen = socket(AF_INET, SOCK_STREAM, 0);
    bind(s_listen, (sockaddr*)&addr_info, sizeof(addr_info));
    listen(s_listen, SOMAXCONN);

    cout << "Waiting for client connection...\n";
    SOCKET s_for_connect = accept(s_listen, (sockaddr*)&addr_info, &size_addr);

    if (s_for_connect != 0) {
        cout << "Client connected.\n";

        while (true) {
            cout << "--- MENU ---\n";
            cout << "1. Receive file from client\n";
            cout << "2. Send file to client\n";
            cout << "3. Exit\n";
            cout << "Enter your choice: ";

            int choice;
            cin >> choice;

            if (choice == 1) {
                recv_file(&s_for_connect);
            }
            else if (choice == 2) {
                send_file(&s_for_connect);
            }
            else if (choice == 3) {
                cout << "Shutting down the server.\n";
                break;
            }
            else {
                cout << "Invalid choice. Try again.\n";
            }
        }
    }
    else {
        cout << "Client connection error.\n";
    }

    closesocket(s_listen);
    closesocket(s_for_connect);
    WSACleanup();
    return 0;
}
