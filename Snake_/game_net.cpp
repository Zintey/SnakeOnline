#include "game_net.h"

// GameServer 
void GameServer::create_room(int port, std::function<void(std::string)> connect_callback, std::function<void()> after_listen)
{
	std::atomic<bool> can_join = true;
	init("0.0.0.0", 8080);
	std::thread room_search_server_thread([&can_join]() {
		UdpServer room_search_server;
		room_search_server.init("0.0.0.0", 8889);
		room_search_server.set_timeout(500);
		while (can_join)
		{
			auto [type, body, from, flag]
				= room_search_server.receive_msg<RoomMsg>();
			if (flag <= 0) continue;
			if (type == MsgType::FindRoom)
			{
				LOG_INFO("received FindRoom from " + std::string(inet_ntoa(from.addr.sin_addr)) + "\n");
				if (std::string(body.version) != GAMEVERSION)
				{
					LOG_INFO("client version mismatch: " + std::string(body.version) + " vs " + GAMEVERSION + "\n");
					RoomMsg reply{ GAMEVERSION, false };
					room_search_server.send_msg<RoomMsg>(MsgType::RoomInfo, reply, from);
					continue;
				}
				RoomMsg reply{ GAMEVERSION , true};
				room_search_server.send_msg<RoomMsg>(MsgType::RoomInfo, reply, from);
			}
		}
		});
	std::thread listen_server_thread([this, &connect_callback]() {
		start_listen([&connect_callback, this](std::string ip, uint8_t index, SOCKET client_socket) {
			connect_callback(ip);
			PlayerIdMsg id_msg{ index };
			send_msg(client_socket, MsgType::PlayerIDInfo, id_msg);
			});
		});

	Sleep(10);
	after_listen();

	while (can_join)
	{
		char c = _getch();
		if (c == 13) // Enter
		{
			can_join = false;
			break;
		}
	}

	close_listen();
	room_search_server_thread.join();
	listen_server_thread.join();
}

void GameServer::start_game()
{
	srand(time(0));
	StartGameMsg start_msg{(unsigned int)rand(), get_clients_size()};
	send_to_all<StartGameMsg>(MsgType::StartGame, start_msg);
	close_listen();
	LOG_INFO("game start\n");
}

// GameClient й╣ож
std::vector<std::string> GameClient::list_room()
{
	room_list.clear();
	std::vector<std::string> ips;

	UdpClient udp_client;
	if (udp_client.init() == -1) return ips;
	udp_client.set_broadcast();
	udp_client.set_timeout(2000);

	SockData broad_sock = create_broadsock(8889);

	RoomMsg find_msg{};
	strcpy_s(find_msg.version, sizeof(find_msg.version), GAMEVERSION);
	find_msg.flag = true;
	udp_client.send_msg(MsgType::FindRoom, find_msg, broad_sock);

	while (true)
	{
		auto [type, body, from, flag] = udp_client.receive_msg<RoomMsg>();
		if (flag <= 0) break;

		if (type == MsgType::RoomInfo )
		{
			if (body.flag)
			{
				room_list.push_back(from);
				ips.push_back(inet_ntoa(from.addr.sin_addr));
			}
			else
			{
				LOG_INFO("received RoomInfo with version mismatch from " + std::string(inet_ntoa(from.addr.sin_addr)) + "\n");
				continue;
			}
		}
	}
	return ips;
}

bool GameClient::join_room(int index)
{
	if (index < 0 || index >= room_list.size()) return false;

	std::string server_ip = inet_ntoa(room_list[index].addr.sin_addr);

	if (this->init() == -1) return false;

	if (this->start_connect(server_ip.c_str(), 8080) == -1) return false;

	auto [type, body] = this->receive_msg<PlayerIdMsg>();
	if (type != MsgType::PlayerIDInfo)
	{
		LOG_ERR("expected PlayerIDInfo, got " + std::to_string((uint16_t)type) + "\n");
		return false;
	}
	else 
	{
		player_id = body.player_id;
		LOG_INFO("joined room successfully, player id: " + std::to_string(player_id) + "\n");
	}
	return true;
}

bool GameClient::wait_start()
{
	while (true)
	{
		auto [type, body] = this->receive_msg<StartGameMsg>();
		if (type == MsgType::StartGame)
		{
			all_player_cnt = body.all_player_cnt;
			game_seed = body.game_seed;
			LOG_INFO("StartGame received, starting game\n");
			return true;
		}
	}
	return false;
}