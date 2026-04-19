#pragma once
#include "online.h"
#include "mapinfo.h"
#include "protocol.h"
#include <atomic>
#include <conio.h>
#include <string>
#include <vector>
#include <mutex>

#define GAMEVERSION ("0.1")

class GameServer : public TcpServer {
public:
	GameServer() = default;
	~GameServer() = default;
	void create_room(int port, std::function<void(std::string)> connect_callback, std::function<void()> after_listen = []() {});
	void start_game();
	void update_game();
public:
	std::mutex mtx;
	char input_buf[4] = { 0 };
};

class GameClient : public TcpClient {
public:
	GameClient() = default;
	~GameClient() = default;

	std::vector<std::string> list_room();
	bool join_room(int index);
	bool wait_start();
public:
	uint8_t get_player_id() const { return player_id; }
	uint8_t get_all_player_cnt() const { return all_player_cnt; }
	unsigned int get_game_seed() const { return game_seed; }
	const std::vector<PlayerConfig>& get_players_config() const { return players_config; }
private:
	std::vector<SockData> room_list;
	uint8_t player_id = -1;
	uint8_t all_player_cnt = 0;
	unsigned int game_seed = 0;
	std::vector<PlayerConfig> players_config;
};