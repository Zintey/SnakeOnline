#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <algorithm>
#include <ws2tcpip.h>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <thread>
#include <functional>
#include <cstring>       
#include "Logger.h"
#include "protocol.h"
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
			return LOG_ERR("tcp listen socket create failed! errcode:" + std::to_string(WSAGetLastError())), -1;

		if (-1 == bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)))
			return closesocket(listen_socket),
			LOG_ERR("bind listen socket failed! errcode:" + std::to_string(WSAGetLastError())), -1;

		return 1;
	}
	void start_listen(std::function<void(std::string, uint8_t, SOCKET)> callback)
	{
		if (-1 == listen(listen_socket, 10))
			return closesocket(listen_socket),
			LOG_ERR("listen failed! errcode:" + std::to_string(WSAGetLastError())), void();
		
		LOG_INFO("listen to port " + std::to_string(ntohs(server_addr.sin_port)));
		while (listen_socket != INVALID_SOCKET)
		{
			auto client = std::make_unique<ClientData>();
			client->socket = accept(listen_socket, (sockaddr*)&client->addr, &client->addrlen);
			if (INVALID_SOCKET == client->socket)
				continue;

			LOG_INFO(std::string(inet_ntoa(client->addr.sin_addr)) + " tcp connect success.\n");
			//std::string buf = "tcp connect success";
			//send(client->socket, buf.c_str(), buf.length(), 0);
			in_addr client_ip = client->addr.sin_addr;
			clients.push_back(std::move(client));
			callback(std::string(inet_ntoa(client_ip)), clients.size() - 1, clients.back()->socket);
		}

	}
	template<typename T>
	void send_to_all(MsgType type, T body)
	{
		for (auto& c : clients)
		{
			if (c == nullptr) continue;
			if (-1 == send_msg(c->socket, type, body))
			{
				LOG_ERR(std::string(inet_ntoa(c->addr.sin_addr)) + " lose connect!!!\n");
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
	template<typename T>
	int send_msg(SOCKET& socket,MsgType type, T body)
	{
		MsgHeader header;
		header.type = (uint16_t)type;
		header.body_len = sizeof(T);
		char* buf = new char[sizeof(MsgHeader) + sizeof(T)];
		memcpy(buf, &header, sizeof(MsgHeader));
		memcpy(buf + sizeof(MsgHeader), &body, sizeof(T));
		int flag = send(socket, buf, sizeof(MsgHeader) + sizeof(T), 0);
		delete[] buf;
		if (flag == -1)
			LOG_ERR("send msg failed! errcode:" + std::to_string(WSAGetLastError()) + '\n');
		return flag;
	}
	void close_listen()
	{
		closesocket(listen_socket);
		listen_socket = INVALID_SOCKET;
	}
	template<typename T>
	std::tuple<MsgType, T> receive_msg(SOCKET& socket)
	{
		char buf[1024] = { 0 };
		int flag = recv(socket, buf, sizeof(buf), 0);
		if (flag <= 0)
			return { MsgType::None, T() };
		MsgHeader header;
		memcpy(&header, buf, sizeof(MsgHeader));
		T body;
		memcpy(&body, buf + sizeof(MsgHeader), header.body_len);
		return { (MsgType)header.type, body };
	}	
	in_addr get_server_addr() const { return server_addr.sin_addr; }
	uint8_t get_clients_size() const { return clients.size();  }
private:
	using ClientData = SockData;
	SOCKET listen_socket = INVALID_SOCKET;
	std::vector<std::unique_ptr<ClientData>> clients;
	sockaddr_in server_addr{};
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
		client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == client_socket)
			return LOG_ERR("client socket create failed! errcode:" + std::to_string(WSAGetLastError()) + '\n'), -1;
		return 1;
	}
	int start_connect(const char* ip, int port)
	{
		target_addr.sin_family = AF_INET;
		target_addr.sin_port = htons(port);
		target_addr.sin_addr.s_addr = inet_addr(ip);
		if (-1 == connect(client_socket, (struct sockaddr*)&target_addr, sizeof(target_addr)))
			return LOG_ERR("connect failed! errcode:" + std::to_string(WSAGetLastError()) + '\n'), -1;
		//char buf[1024] = { 0 };
		//recv(client_socket, buf, 1024, 0);
		//std::cout << buf << '\n';
		return 1;
	}
	//int send_msg(std::string buf)
	//{
		//return send(client_socket, buf.c_str(), buf.length(), 0);
	//}
	template<typename T>
	int send_msg(MsgType type, T body)
	{
		MsgHeader header;
		header.type = (uint16_t)type;
		header.body_len = sizeof(T);
		char* buf = new char[sizeof(MsgHeader) + sizeof(T)];
		memcpy(buf, &header, sizeof(MsgHeader));
		memcpy(buf + sizeof(MsgHeader), &body, sizeof(T));
		int flag = send(client_socket, buf, sizeof(MsgHeader) + sizeof(T), 0);
		delete[] buf;
		if (flag == -1)
			LOG_ERR("send msg failed! errcode:" + std::to_string(WSAGetLastError()) + '\n');
		return flag;
	}
	//std::string receive()
	//{
		//char buf[1024] = { 0 };
		//recv(client_socket, buf, sizeof(buf), 0);
		//return buf;
	//}
	template<typename T>
	std::tuple<MsgType, T> receive_msg()
	{
		char buf[1024] = { 0 };
		int flag = recv(client_socket, buf, sizeof(buf), 0);
		if (flag <= 0)
			return { MsgType::None, T() };
		MsgHeader header;
		memcpy(&header, buf, sizeof(MsgHeader));
		T body;
		memcpy(&body, buf + sizeof(MsgHeader), header.body_len);
		return { (MsgType)header.type, body };
	}
private:
	SOCKET client_socket = INVALID_SOCKET;
	sockaddr_in target_addr{};
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
	void set_timeout(int timeout)
	{
		setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	}
	int init()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 2), &wsa_data);
		server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (INVALID_SOCKET == server_socket)
			return LOG_ERR("udp server socket create failed! errcode:" + std::to_string(WSAGetLastError()) + '\n'), -1;

		if (-1 == bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)))
			return closesocket(server_socket),
			LOG_ERR("bind udp server socket failed! errcode:" + std::to_string(WSAGetLastError())  + '\n'), -1;
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
	template<typename T>
	std::tuple<MsgType, T, SockData, int > receive_msg()
	{
		char buf[1024] = { 0 };
		SockData c;
		int flag = recvfrom(server_socket, buf, sizeof(buf), 0,
			(sockaddr*)&c.addr, &c.addrlen);
		if (flag <= 0)
			return { MsgType::None, T(), c, flag};
		MsgHeader header;
		memcpy(&header, buf, sizeof(MsgHeader));
		T body;
		memcpy(&body, buf + sizeof(MsgHeader), header.body_len);
		return { (MsgType)header.type, body, c, flag};
	}
	//int send(std::string buf, ClientData c) {
	//	return sendto(server_socket, buf.c_str(), buf.length(), 0, (const sockaddr*)&c.addr, c.addrlen);
	//}
	template<typename T>
	int send_msg(MsgType type, T body, const SockData& target)
	{
		MsgHeader header;
		header.type = (uint16_t)type;
		header.body_len = sizeof(T);
		char* buf = new char[sizeof(MsgHeader) + sizeof(T)];
		memcpy(buf, &header, sizeof(MsgHeader));
		memcpy(buf + sizeof(MsgHeader), &body, sizeof(T));
		int flag = sendto(server_socket, buf, sizeof(MsgHeader) + sizeof(T), 0,
			(const sockaddr*)&target.addr, target.addrlen);
		delete[] buf;
		if (flag == -1)
			LOG_ERR("send msg failed! errcode:" + std::to_string(WSAGetLastError()) + '\n');
		return flag;
	}
	in_addr get_server_addr() const { return server_addr.sin_addr; }
private:
	SOCKET server_socket = INVALID_SOCKET;
	sockaddr_in server_addr{};
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
			return LOG_ERR("udp server socket create failed! errcode:" + std::to_string(WSAGetLastError()) + '\n'), -1;

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
	template<typename T>
	std::tuple<MsgType, T, SockData, int > receive_msg()
	{
		char buf[1024] = { 0 };
		SockData c;
		int flag = recvfrom(client_socket, buf, sizeof(buf), 0,
			(sockaddr*)&c.addr, &c.addrlen);
		if (flag <= 0)
			return { MsgType::None, T(), c, flag };
		MsgHeader header;
		memcpy(&header, buf, sizeof(MsgHeader));
		T body;
		memcpy(&body, buf + sizeof(MsgHeader), header.body_len);
		return { (MsgType)header.type, body, c, flag };
	}
	template<typename T>
	int send_msg(MsgType type, T body, const SockData& target)
	{
		MsgHeader header;
		header.type = (uint16_t)type;
		header.body_len = sizeof(T);
		char* buf = new char[sizeof(MsgHeader) + sizeof(T)];
		memcpy(buf, &header, sizeof(MsgHeader));
		memcpy(buf + sizeof(MsgHeader), &body, sizeof(T));
		int flag = sendto(client_socket, buf, sizeof(MsgHeader) + sizeof(T), 0,
			(const sockaddr*)&target.addr, target.addrlen);
		delete[] buf;
		return flag;
	}
	//int send(std::string buf, SockData c) {
	//	return sendto(client_socket, buf.c_str(), buf.length(), 0, (const sockaddr*)&c.addr, c.addrlen);
	//}
	
private:
	SOCKET client_socket = INVALID_SOCKET;
	sockaddr_in target_addr{};
};

inline const SockData create_broadsock(int port)
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