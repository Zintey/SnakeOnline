#pragma once
//#include <winsock2.h>
//#include <ws2tcpip.h>
 //#include <WinSock2.h>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <thread>
#pragma comment(lib, "ws2_32.lib")


struct SockData {
	SOCKET socket = INVALID_SOCKET;
	sockaddr_in addr{};
	int addrlen = sizeof(sockaddr_in);
};

class TcpServer {
public:
	TcpServer() = default;
	TcpServer(const char* ip, int port) {
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		server_addr.sin_addr.s_addr = inet_addr(ip);
	}
	~TcpServer() {
		closesocket(listen_socket);
		WSACleanup();
	}
	int init(const char* ip, int port) {
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		server_addr.sin_addr.s_addr = inet_addr(ip);
		return init();
	}
	int init()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == listen_socket)
			return std::cerr << "tcp listen socket create failed! errcode:" << GetLastError() << std::endl, -1;

		if (-1 == bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)))
			return closesocket(listen_socket),
			std::cerr << "bind listen socket failed! errcode:" << GetLastError() << std::endl, -1;

		return 1;
	}
	void start_listen()
	{
		if (-1 == listen(listen_socket, 10))
			return closesocket(listen_socket),
			std::cerr << "listen failed! errcode:" << GetLastError() << std::endl, void();
		else
			std::cout << "listen to port " << ntohs(server_addr.sin_port) << std::endl;
		while (listen_socket != INVALID_SOCKET)
		{
			auto client = std::make_unique<ClientData>();
			client->socket = accept(listen_socket, (sockaddr*)&client->addr, &client->addrlen);
			if (INVALID_SOCKET == client->socket)
				continue;

			std::cout << inet_ntoa(client->addr.sin_addr) << " tcp connect success.\n";
			std::string buf = "tcp connect success";
			send(client->socket, buf.c_str(), buf.length(), 0);

			clients.push_back(std::move(client));
		}

	}
	void send_to_all(std::string buf)
	{
		for (auto& c : clients)
		{
			if (c == nullptr) continue;
			std::cout << "send to " << inet_ntoa(c->addr.sin_addr) << " ...\n";
			if (-1 == send(c->socket, buf.c_str(), buf.length(), 0))
			{
				std::cerr << inet_ntoa(c->addr.sin_addr) << " lose connect!!!\n";
				closesocket(c->socket);
				c = nullptr;
				continue;
			}
		}
		clients.erase(std::remove_if(clients.begin(), clients.end(),
			[](auto& c) {return c == nullptr; }),
			clients.end()
		);
	}
	std::string receive()
	{
		char buf[1024] = { 0 };
		// recv(client_socket, buf, sizeof(buf), 0);
		return buf;
	}
	void close_listen()
	{
		closesocket(listen_socket);
	}

private:
	using ClientData = SockData;
	SOCKET listen_socket;
	std::vector<std::unique_ptr<ClientData>> clients;
	sockaddr_in server_addr;
};


class TcpClient {
public:
	TcpClient() = default;
	TcpClient(const char* ip, int port) {
		target_addr.sin_family = AF_INET;
		target_addr.sin_port = htons(port);
		target_addr.sin_addr.s_addr = inet_addr(ip);
	}
	~TcpClient() {
		closesocket(client_socket);
		WSACleanup();
	}
	int init()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == client_socket)
			return std::cerr << "client socket create failed! errcode:" << GetLastError() << '\n', -1;
		return 1;
	}
	int start_connect(const char* ip, int port)
	{
		target_addr.sin_family = AF_INET;
		target_addr.sin_port = htons(port);
		target_addr.sin_addr.s_addr = inet_addr(ip);
		if (-1 == connect(client_socket, (struct sockaddr*)&target_addr, sizeof(target_addr)))
			return std::cerr << "connect failed! errcode:" << GetLastError() << '\n', -1;
		char buf[1024] = { 0 };
		recv(client_socket, buf, 1024, 0);
		std::cout << buf << '\n';
		return 1;
	}
	int send_to(std::string buf)
	{
		return send(client_socket, buf.c_str(), buf.length(), 0);
	}
	std::string receive()
	{
		char buf[1024] = { 0 };
		recv(client_socket, buf, sizeof(buf), 0);
		return buf;
	}
private:
	SOCKET client_socket;
	sockaddr_in target_addr;
};

class UdpServer {
public:
	using ClientData = SockData;
	UdpServer() = default;
	UdpServer(const char* ip, int port) {
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		server_addr.sin_addr.s_addr = inet_addr(ip);
	}
	~UdpServer() {
		closesocket(server_socket);
		WSACleanup();
	}
	int init(const char* ip, int port) {
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		server_addr.sin_addr.s_addr = inet_addr(ip);
		return init();
	}
	int init()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (INVALID_SOCKET == server_socket)
			return std::cerr << "udp server socket create failed! errcode:" << GetLastError() << std::endl, -1;

		if (-1 == bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)))
			return closesocket(server_socket),
			std::cerr << "bind udp server socket failed! errcode:" << GetLastError() << std::endl, -1;
		return 1;
	}
	std::tuple<std::string, SockData, int> receive() {
		ClientData c;
		char buf_c[1024] = { 0 };
		int flag = recvfrom(server_socket, buf_c, sizeof(buf_c), 0, (sockaddr*)&c.addr, &c.addrlen);
		std::string buf;
		if (flag > 0) {
			buf.assign(buf_c, flag);
		}
		return { buf, c, flag };
	}
	int send(std::string buf, ClientData c) {
		return sendto(server_socket, buf.c_str(), buf.length(), 0, (const sockaddr*)&c.addr, c.addrlen);
	}
private:
	SOCKET server_socket;
	sockaddr_in server_addr;
};


class UdpClient {
public:
	UdpClient() = default;
	UdpClient(const char* ip, int port) {
		target_addr.sin_family = AF_INET;
		target_addr.sin_port = htons(port);
		target_addr.sin_addr.s_addr = inet_addr(ip);
	}
	~UdpClient() {
		closesocket(client_socket);
		WSACleanup();
	}
	int init(const char* ip, int port) {
		target_addr.sin_family = AF_INET;
		target_addr.sin_port = htons(port);
		target_addr.sin_addr.s_addr = inet_addr(ip);
		return init();
	}
	int init()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (INVALID_SOCKET == client_socket)
			return std::cerr << "udp server socket create failed! errcode:" << GetLastError() << std::endl, -1;

		return 1;
	}
	void set_broadcast()
	{
		int bBroadcast = 1;
		setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (const char*)&bBroadcast, sizeof(bBroadcast));
	}
	void set_timeout(int timeout)
	{
		setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	}
	std::tuple<std::string, SockData, int> receive() {
		SockData c;
		char buf_c[1024] = { 0 };
		int flag = recvfrom(client_socket, buf_c, sizeof(buf_c), 0, (sockaddr*)&c.addr, &c.addrlen);
		std::string buf;
		if (flag > 0) {
			buf.assign(buf_c, flag);
		}
		return { buf, c, flag };
	}
	int send(std::string buf, SockData c) {
		return sendto(client_socket, buf.c_str(), buf.length(), 0, (const sockaddr*)&c.addr, c.addrlen);
	}
private:
	SOCKET client_socket;
	sockaddr_in target_addr;
};

const SockData create_broadsock(int port)
{
	SockData broadsockdata;
	// broadsockdata = (SockData*) operator new(sizeof(SockData));
	// broadsockdata.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// if (broadsockdata.socket == INVALID_SOCKET)
	// 	return std::cerr << "create broadsock failed!", broadsockdata;
	// int bBroadcast = 1;
	// setsockopt(broadsockdata.socket, SOL_SOCKET, SO_BROADCAST, (const char*)&bBroadcast, sizeof(bBroadcast));
	sockaddr_in targetAddr;
	targetAddr.sin_family = AF_INET;
	targetAddr.sin_port = htons(port);
	targetAddr.sin_addr.s_addr = INADDR_BROADCAST;
	broadsockdata.addr = targetAddr;
	return broadsockdata;
}