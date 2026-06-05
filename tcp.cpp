#include "declaration.h"
#include "tcp.h"

// TCP
TCP::TCP() : _sockfd(-1) {}

TCP::~TCP() {
	closeConnection();
}

bool TCP::initTcp(const std::string ip, int port, float _time_out) {
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	std::string _log_msg = "TCP Server ip: ";
	if (_sockfd < 0) {
		std::cout << "TCP Server ip: " << ip << " Failed to create socket" << std::endl;
		return false;
	}

	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(static_cast<uint16_t>(port));
	if (inet_pton(AF_INET, ip.c_str(), &_server_addr.sin_addr) <= 0) {
		std::cout << "TCP Server ip: " << ip << " Invalid address or address not supported" << std::endl;
	}

	if (::connect(_sockfd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) < 0) {
		std::cout << "TCP Server ip: " << ip << " Connection failed, errno: " << std::to_string(errno) << std::endl;
		close(_sockfd);
		_sockfd = -1;
		return false;
	}

	// 設定 socket 超時時間
	struct timeval timeout;
	timeout.tv_sec = static_cast<int>(_time_out);  // 設定超時秒數
	timeout.tv_usec = static_cast<int>((_time_out - timeout.tv_sec) * 1000000);  // 設定超時微秒數
	if (setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
		std::cout << "TCP Server ip: " << ip << " Failed to set timeout" << std::endl;
	}
	return true;
}

void TCP::Send(char* data, size_t length) {
	if (_sockfd == -1) {
		std::cout << "Socket not initialized" << std::endl;
		throw std::runtime_error("Socket not initialized");
	}

	if (::send(_sockfd, data, length, 0) < 0) {
		std::cout << "Send failed" << std::endl;
		throw std::runtime_error("Send failed");
	}
}

void TCP::Send(const std::string& data) {
	if (_sockfd == -1) {
		std::cout << "Socket not initialized" << std::endl;
		throw std::runtime_error("Socket not initialized");
	}

	if (::send(_sockfd, data.c_str(), data.size(), 0) < 0) {
		std::cout << "Send failed" << std::endl;
		throw std::runtime_error("Send failed");
	}
}

int TCP::Recv(char* buffer, int buffer_size) {
	if (_sockfd == -1) {
		std::cout << "Socket not initialized" << std::endl;
		throw std::runtime_error("Socket not initialized");
	}

	bzero(buffer, buffer_size);
	ssize_t bytes_received = recv(_sockfd, buffer, buffer_size, 0);
	if (bytes_received < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			return 0;
		}
		else {
			throw std::runtime_error("Receive failed");
		}
	}

	return static_cast<int>(bytes_received);
}

void TCP::closeConnection() {
	if (_sockfd != -1) {
		close(_sockfd);
		_sockfd = -1;
	}
}

