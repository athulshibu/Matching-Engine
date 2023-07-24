// Running code
// g++ order_generator.cpp -o client.exe -lws2_32 && client.exe

#include <iostream>
#include <winsock2.h>
#include <string>
#include <sstream>
#include <random>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

int chosenPort = 80;
int randomSeed = 227;
int numOrders = 100;

std::string createFormattedString(int a, int b, int c) {
    std::ostringstream oss;
    // oss << "{" << a << ", " << b << ", " << c << "}\n";
    oss << a << b << c << "\n";
    return oss.str();
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(9, 1), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with the server IP address
    serverAddr.sin_port = htons(chosenPort); // Replace with the server port number

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::mt19937 gen(randomSeed);
    std::uniform_int_distribution<int> dis1(0, 1);
    std::uniform_int_distribution<int> dis2(0, 8);
    std::uniform_int_distribution<int> dis3(1, 4);

    std::string orders[numOrders];

    for (int i=0; i<numOrders; i++ ) {
        int num1 = dis1(gen); // Generate random number between specified range (inclusive)
        int num2 = dis2(gen);
        int num3 = dis3(gen);   
        orders[i] = createFormattedString(num1, num2, num3);
    }

    // Sending order messages to the server
    for (int i=0; i<numOrders ; i++) {
        if (send(clientSocket, orders[i].c_str(), orders[i].size(), 0) == SOCKET_ERROR)
            std::cerr << "Failed to send order message 1." << std::endl;
        
        unsigned int type = orders[i][0] - '0';
        double price = 100 + (orders[i][1] - '0') * 0.25;
        int quantity = 25 * (orders[i][2] - '0');
        // std::cout << orders[i];
        std::cout << i+1 << ". { " << (type==1 ? "BUY" : "SELL") << ", " 
                  <<  std::to_string(price) << ", " 
                  << std::to_string(quantity) << " }\n";
        Sleep(1);
        if (i%50 == 0)
            Sleep(1);
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
