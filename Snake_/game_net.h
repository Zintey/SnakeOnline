#pragma once
#include "online.h"
#include "protocol.h"
#include <atomic>
#include <conio.h>
#include <string>
#include <vector>

#define GAMEVERSION ("0.1")

class GameServer : private TcpServer {
public:
	GameServer() = default;
	~GameServer() = default;
	void create_room(int port, std::function<void(std::string)> connect_callback, std::function<void()> after_listen = []() {});
	void start_game();
};

class GameClient : private TcpClient {
public:
	GameClient() = default;
	~GameClient() = default;

	std::vector<std::string> list_room();
	bool join_room(int index);
	bool wait_start();
public:
	unsigned int get_game_seed() const { return game_seed; }
private:
	std::vector<SockData> room_list;
	uint8_t player_id = -1;
	uint8_t all_player_cnt = 0;
	unsigned int game_seed = 0;
};