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
	PlayerID = 3,   
	PlayerInfo = 4,
	Input = 5,
};

struct RoomMsg {
	char version[16];
	bool flag;
};

struct PlayerIDMsg {
	uint8_t player_id;
};

struct PlayerConfig {
	uint8_t player_id;
	int spawn_x;
	int spawn_y;
	WORD color;
	WORD eyes_color;
	int dir;
};

struct PlayerInfoMsg {
	unsigned int game_seed;
	uint8_t all_player_cnt;
	PlayerConfig players[4];
};


struct InputMsg {
	uint8_t player_id;
	char input_char;
};

struct AllPlayerInputMsg {
	char inputs[4];
};
#pragma pack(pop)