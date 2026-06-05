#pragma once

class TCP {
private:
    int _sockfd;
    struct sockaddr_in _server_addr;
public:
    TCP();
    ~TCP();

    bool initTcp(const std::string ip, int port, float time_out);
    void Send(char* data, size_t length);
    void Send(const std::string& data);
    int  Recv(char* buffer, int buffer_size);
    bool isConnected() const { return _sockfd >= 0; }
    void closeConnection();
};
