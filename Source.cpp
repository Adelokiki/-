#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include<iostream>
#include<fstream>
#include<string>
#include<experimental/filesystem>
#pragma warning(disable:4996)
#pragma comment (lib, "ws2_32.lib")
#include<WinSock2.h>

using namespace std;

bool file_ready_for_confirmation = false;

void send_file(SOCKET* sock) {
    string file_name;
    cout << "Enter the name of the file to send to the server: ";
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

        cout << "File \"" << file_name << "\" successfully sent to the server.\n";
        file_ready_for_confirmation = true;

        delete[] bytes;
    }
    else {
        cout << "Error: file not found.\n";
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
        cout << "File \"" << file_name << "\" successfully received from the server.\n";
        file_ready_for_confirmation = true;
    }
    else {
        cout << "Error saving file.\n";
    }

    delete[] bytes;
    file.close();
}


int main() {
    cout << "=== CLIENT ===" << endl;

    WORD dllVer = MAKEWORD(2, 1);
    WSAData wsad;
    WSAStartup(dllVer, &wsad);

    SOCKADDR_IN addr_info;
    memset(&addr_info, 0, sizeof(SOCKADDR_IN));

    addr_info.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addr_info.sin_port = htons(4321);
    addr_info.sin_family = AF_INET;

    SOCKET s_client = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(s_client, (sockaddr*)&addr_info, sizeof(addr_info)) == 0) {
        cout << "Connection to the server established.\n";

        while (true) {
            cout << "--- MENU ---\n";
            cout << "1. Send a file to the server\n";
            cout << "2. Receive a file from the server\n";
            cout << "3. Exit\n";
            cout << "Enter your choice: ";

            int choice;
            cin >> choice;

            if (choice == 1) {
                send_file(&s_client);
            }
            else if (choice == 2) {
                recv_file(&s_client);
            }
            else if (choice == 3) {
                cout << "Client shutting down.\n";
                break;
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }
        }
    }
    else {
        cout << "Error connecting to the server.\n";
    }

    closesocket(s_client);
    WSACleanup();
    return 0;
}
