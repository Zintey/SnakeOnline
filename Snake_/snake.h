#ifndef _SNAKE_H_
#define _SNAKE_H_

#include <stdlib.h>
#include <vector>
#include "vector2.h"
#include "timer.h"
#include "mapinfo.h"
#include "player_id.h"
#include "canvas.h"

extern MapItemType map[HEIGHT][WIDTH];

struct Node {
	Vector2 position;
	Node* next_node;
	Node* prev_node;
};

enum class Direction
{
	Left,
	Right,
	Up,
	Down
};

class DoubleBuffer;

class Snake {
public:
	Snake(PlayerID id, int len, Direction dir, float speed, Vector2 position,
		int opt_up, int opt_down, int opt_left, int opt_right, int opt_accelerate,
		WORD eyes_color, WORD body_color, WORD body_accelerate_color);

	~Snake();

	void on_input(int opt);

	void on_update(int delta);

	void on_draw(Canvas* canvas, Vector2 anchor);

	void on_move();

	bool check_is_die() const;

	bool check_is_lengthen() const;

	bool check_is_accelerate() const;

	int get_score() const;

	const WORD& get_body_color() const;

	Direction get_dir();
private:
	const int node_pool_size = HEIGHT * WIDTH;
	const int input_cd = 80;
	const int accelerate_during_time = 300;
private:
	int dst_len = 0;
	int real_len = 0;
	Direction input_dir = Direction::Right;
	Direction dir = Direction::Right;
	bool is_input = false;

	float speed = 0;
	Timer timer_move;

	Timer timer_accelerate;
	bool is_accelerate = false;

	Node* head_node = nullptr;
	Node* tail_node = nullptr;
	
	Node* node_pool[HEIGHT * WIDTH];
	int node_pool_top = -1;
	
	int is_lengthen = false;
	Timer timer_lengthen;

	float score = 0;

	Timer timer_time_score;

	bool is_die = false;

	int opt_char[5] = {'w', 's', 'a', 'd', ' '};
#define OPT_UP opt_char[0]
#define OPT_DOWN opt_char[1]
#define OPT_LEFT opt_char[2]
#define OPT_RIGHT opt_char[3]
#define OPT_ACCELETATE opt_char[4]

	PlayerID id = PlayerID::P1;

	WORD eyes_color = FG_RED;
	WORD body_color = BG_YELLOW;
	WORD body_accelerate_color = BG_DARK_YELLOW;
	WORD body_lengthen_color = BG_GREEN;

private:
	bool is_can_move(Vector2 position);
	Vector2 get_next_position();

private:
	int speed_to_wait_time(float speed);
	void on_lengthen(WORD score_color);
	void on_accelerate();
	void stop_accelerate();
	void eat_score(Vector2 position);

private:
	Node* get_new_node();
	void delete_node(Node* node);
};
#endif