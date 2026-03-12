#pragma once

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <memory>
#include <functional>
#include <atomic>

class UDPClient{
public:
    UDPClient(const char* serverIP, int serverPort)
    {
        clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientSocket == -1) {
            std::cerr << "Error creating socket\n";
            return;
        }
        std::memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverIP, &serverAddress.sin_addr);
    }
    ~UDPClient()
    {
        close(clientSocket);
    }
    void sendMessage(const char* message)
    {
        sendto(clientSocket, message, std::strlen(message), 0,
           (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    }
private:
    int clientSocket;
    struct sockaddr_in serverAddress;
};



using UdpReceiveCallback = std::function<void(const uint8_t*, size_t, const sockaddr_in&)>;

class UDPServer {
public:
    UDPServer(int port) : stop_flag_(false) {
        serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (serverSocket == -1) {
            std::cerr << "Error creating socket\n";
            return;
        }

        // 设置地址复用（可选，防止重启时报 Address already in use）
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        std::memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "Error binding socket\n";
            close(serverSocket);
            return;
        }

        std::cout << "UDP Server started on port " << port << std::endl;

        // 启动接收线程
        _thread_ = std::make_shared<std::thread>(&UDPServer::Start, this);
    }

    ~UDPServer() {
        Stop();
        if (_thread_ && _thread_->joinable()) {
            _thread_->join();
        }
        if (serverSocket != -1) {
            close(serverSocket);
        }
    }

    // 回调函数
    void setCallback(UdpReceiveCallback callback) {
        callback_ = std::move(callback);
    }

    // 停止线程
    void Stop() {
        stop_flag_.store(true);
        // 唤醒阻塞的 recvfrom (通过发送一个包给自己或 shutdown，这里简单起见依赖超时或外部中断，
        // 生产环境建议设置 socket 超时或使用 socketpair 唤醒)
        // 简易做法：关闭 socket 会导致 recvfrom 返回错误，从而退出循环
        if (serverSocket != -1) {
            shutdown(serverSocket, SHUT_RD); 
        }
    }

private:
    void Start() {
        char buffer[1024];
        struct sockaddr_in clientAddress;
        socklen_t clientAddrLen = sizeof(clientAddress);

        while (!stop_flag_.load()) {
            // 注意：如果没有设置超时，shutdown/close socket 是退出循环的关键
            ssize_t bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                                            (struct sockaddr*)&clientAddress, &clientAddrLen);

            if (bytesReceived == -1) {
                if (errno == EINTR || errno == EBADF) { 
                    // 被信号中断或 socket 已关闭，正常退出
                    break; 
                }
                std::cerr << "Error receiving data: " << strerror(errno) << "\n";
                continue;
            }

            // 回调已设置，则调用它
            if (callback_) {
                // 将 char* 转换为 uint8_t* 并调用回调
                callback_(reinterpret_cast<const uint8_t*>(buffer), 
                          static_cast<size_t>(bytesReceived), 
                          clientAddress);
            } else {
                std::cout << "Received (No Callback): " << std::string(buffer, bytesReceived) << std::endl;
            }
        }
    }

private:
    int serverSocket = -1;
    struct sockaddr_in serverAddress{};
    std::shared_ptr<std::thread> _thread_ = nullptr;
    UdpReceiveCallback callback_; // 存储回调
    std::atomic<bool> stop_flag_;
};