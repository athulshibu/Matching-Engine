// Running code
// g++ matching_engine.cpp -o server.exe -lws2_32 && server.exe

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <winsock2.h>
#include <windows.h>
#include <algorithm>
#pragma comment(lib, "ws2_32.lib")

int chosenPort = 80;
int buyIDcount = 1;
int sellIDcount = 1;
double matchingEngineProfit = 0;

struct Order {
    int id;
    unsigned int type;
    double price;
    int quantity;
    time_t timestamp;

    Order(unsigned int ordertype, double orderprice, int orderquantity)
        : type(ordertype), price(orderprice), quantity(orderquantity) {
        if (type == 1)
            id = buyIDcount++;
        else if (type == 0)
            id = sellIDcount++;
        timestamp = std::time(0);               // Timestamp assigned by system at the time of order placement
    }

    std::string to_string() const {
      return "{ " + std::to_string(id) + ", " + (type==1 ? "BUY" : "SELL") + ", " +  std::to_string(price) + ", " + std::to_string(quantity) + " }\n";
  }
};

struct Trade {
    int buyOrderId;
    int sellOrderId;
    double price;
    int quantity;
    time_t timestamp;

    Trade(int buyOrderId, int sellOrderId, double price, int quantity)
        : buyOrderId(buyOrderId), sellOrderId(sellOrderId), price(price), quantity(quantity) {
            timestamp = std::time(0);
        }
};

class OrderMatchingEngine {
public:
    void addOrder(Order order) {
        if (order.type == 1) {
            buyOrders.push_back(order);
        } else if (order.type == 0) {
            sellOrders.push_back(order);
        }
        matchOrders();
    }

    void matchOrders() {
        // std::sort(buyOrders.begin(), buyOrders.end(), [](const Order& a, const Order& b) {
        //     return a.price > b.price;
        // });

        // std::sort(sellOrders.begin(), sellOrders.end(), [](const Order& a, const Order& b) {
        //     return a.price < b.price;
        // });

        while (!buyOrders.empty() && !sellOrders.empty()) {
            Order& buyOrder = buyOrders.back();
            Order& sellOrder = sellOrders.back();

            // if (buyOrder.price == sellOrder.price) {
            //     int quantity = std::min(buyOrder.quantity, sellOrder.quantity);
            //     trades.emplace_back(buyOrder.id, sellOrder.id, sellOrder.price, quantity);

            //     buyOrder.quantity -= quantity;
            //     sellOrder.quantity -= quantity;

            //     if (buyOrder.quantity <= 0)
            //         buyOrders.pop_back();
            //     if (sellOrder.quantity <= 0)
            //         sellOrders.pop_back();
            // } 

            if (buyOrder.price >= sellOrder.price) {
                int quantity = std::min(buyOrder.quantity, sellOrder.quantity);
                trades.emplace_back(buyOrder.id, sellOrder.id, sellOrder.price, quantity);

                buyOrder.quantity -= quantity;
                sellOrder.quantity -= quantity;

                matchingEngineProfit += ((buyOrder.price - sellOrder.price) * quantity);

                if (buyOrder.quantity <= 0)
                    buyOrders.pop_back();
                if (sellOrder.quantity <= 0)
                    sellOrders.pop_back();
            }
            
            else
                break;
        }
    }

    int numTrades() {
        int count = 0;
        for (const Trade& trade : trades)
            count++;
        return count;
    }

    void printTrades(int numToPrint) {
        for (int i=numToPrint ; i < trades.size() ; i++) {
            std::cout << i+1 << ". "
                      << "Buy ID: " << trades[i].buyOrderId << ", "
                      << "\tSell ID: " << trades[i].sellOrderId << ", "
                      << "\tPrice: " << trades[i].price << ", "
                      << "\tQuantity: " << trades[i].quantity << ", "
                      << "\tTimestamp: " << std::ctime(&trades[i].timestamp);
            if (i%100 == 0) {
                std::cout<< "Total Trades = " << numTrades() << std::endl;
                std::cout<< "Engine Profit = " << matchingEngineProfit << std::endl;     
            }
        }
    }

private:
    std::vector<Order> buyOrders;
    std::vector<Order> sellOrders;
    std::vector<Trade> trades;
};

class TCPServer {
public:
    TCPServer(int port) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(9, 1), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock." << std::endl;
            return;
        }

        listenSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listenSocket_ == INVALID_SOCKET) {
            std::cerr << "Failed to create listen socket." << std::endl;
            WSACleanup();
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        // serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port);

        int reuseAddr = 1;
        if (setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
            std::cerr << "Failed to set SO_REUSEADDR option." << std::endl;
            closesocket(listenSocket_);
            WSACleanup();
            return;
        }

        if (bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind the listen socket."  << WSAGetLastError() << std::endl;
            closesocket(listenSocket_);
            WSACleanup();
            return;
        }

        if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Failed to start listening."  << WSAGetLastError() << std::endl;
            closesocket(listenSocket_);
            WSACleanup();
            return;
        }
    }

    ~TCPServer() {
        closesocket(listenSocket_);
        std::cout<< "Engine Profit = " << matchingEngineProfit << std::endl;
        WSACleanup();
    }

    void acceptConnections() {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        while (true) {
            SOCKET clientSocket = accept(listenSocket_, (sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Failed to accept connection." << WSAGetLastError() << std::endl;
                continue;
            }

            HANDLE threadHandle = CreateThread(NULL, 0, handleClientWrapper, reinterpret_cast<LPVOID>(clientSocket), 0, NULL);
            if (threadHandle == NULL) {
                std::cerr << "Failed to create thread." << std::endl;
            } 
            else
                CloseHandle(threadHandle); // Close the thread handle as we won't wait for the thread to finish
        }
    }

private:
    static DWORD WINAPI handleClientWrapper(LPVOID lpParam) {
        SOCKET clientSocket = reinterpret_cast<SOCKET>(lpParam);
        TCPServer server(chosenPort);
        server.handleClient(clientSocket);
        return 0;
    }

    void handleClient(SOCKET clientSocket) {
        char buffer[1024];
        int bytesRead;
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            
            // Process the received data
            std::string message(buffer, bytesRead);

            // Parse the message to extract order details (e.g., ID, Type, Price, Quantity)
            // Create an Order object and add it to the matching engine
            unsigned int type = message[0] - '0';
            double price = 100 + (message[1] - '0') * 0.25;
            int quantity = 25 * (message[2] - '0');

            Order order(type, price, quantity);
            matchingEngine.addOrder(order);

            int count = matchingEngine.numTrades();
            if (numTrades < count){
                matchingEngine.printTrades(numTrades);
                numTrades = count;
            }
        }

        closesocket(clientSocket);
    }

    SOCKET listenSocket_;
    OrderMatchingEngine matchingEngine;
    int numTrades = 0;
};

int main() {
    try {
        TCPServer server(chosenPort); 
        server.acceptConnections();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}