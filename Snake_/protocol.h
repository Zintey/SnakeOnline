#pragma once
#include <cstdint>
#include <WinSock2.h>
#pragma pack(push, 1)

struct MsgHeader {
	uint16_t type;
	uint16_t body_len;
};

enum class MsgType : uint16_t {
	None = 0,
	FindRoom = 1,
	RoomInfo = 2,
	PlayerIDInfo = 3,
	StartGame = 4
};

struct RoomMsg {
	char version[16];
	bool flag;
};

struct PlayerIdMsg {
	uint8_t player_id;
};

struct StartGameMsg {
	unsigned int game_seed;
	uint8_t all_player_cnt;
};

#pragma pack(pop)
