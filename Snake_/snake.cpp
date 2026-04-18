#include "snake.h"
#include "score_spawner.h"

extern std::vector<ScoreSpawner*> score_spawner_list;

Snake::Snake(PlayerID id, int len, Direction dir, float speed, Vector2 position, 
	int opt_up, int opt_down, int opt_left, int opt_right, int opt_accelerate,
	WORD eyes_color = FG_RED, WORD body_color = BG_YELLOW, WORD body_accelerate_color = BG_DARK_YELLOW)
	: id(id), dst_len(len), dir(dir), speed(speed), 
	eyes_color(eyes_color), body_color(body_color), body_accelerate_color(body_accelerate_color)
{
	OPT_UP = opt_up;
	OPT_DOWN = opt_down;
	OPT_LEFT = opt_left;
	OPT_RIGHT = opt_right;
	OPT_ACCELETATE = opt_accelerate;

	Node* node = new Node();
	node->position = position;
	node->next_node = nullptr;
	node->prev_node = nullptr;

	this->head_node = node;
	this->tail_node = node;
	this->real_len = 1;
	
	node_pool_top = -1;
	for (int i = 0; i < node_pool_size; i++)
		node_pool[++node_pool_top] = new Node();

	timer_move.set_loop(true);
	
	timer_move.set_wait_time(speed_to_wait_time(speed));
	timer_move.set_callback([&]() {
		on_move();
		});

	timer_lengthen.set_loop(false);
	timer_lengthen.set_wait_time(1500);
	timer_lengthen.set_callback([&]() {
		is_lengthen = false;
		});

	timer_accelerate.set_loop(false);
	timer_accelerate.set_wait_time(accelerate_during_time);
	timer_accelerate.set_callback([&]() {
		stop_accelerate();
		});

	timer_time_score.set_loop(true);
	timer_time_score.set_wait_time(10000);
	timer_time_score.set_callback([&]() {
		score += 20;
		});
}

Snake::~Snake()
{
	Node* u = head_node;
	while (u)
	{
		Node* next = u->next_node;
		delete u;
		u = next;
	}
	head_node = nullptr;
	for (int i = 0; i <= node_pool_top; i++)
		delete node_pool[i];

}

void Snake::on_input(int opt)
{
	if (is_die) return;
	if (opt == OPT_UP)
	{
		if (dir != Direction::Down)
			input_dir = Direction::Up, is_input = true;
	}
	else if (opt == OPT_DOWN)
	{
		if (dir != Direction::Up)
			input_dir = Direction::Down, is_input = true;
	}
	else if (opt == OPT_LEFT)
	{
		if (dir != Direction::Right)
			input_dir = Direction::Left, is_input = true;
	}
	else if (opt == OPT_RIGHT)
	{
		if (dir != Direction::Left)
			input_dir = Direction::Right, is_input = true;
	}
	else if (opt == OPT_ACCELETATE)
		on_accelerate();
}

void Snake::on_update(int delta)
{
	if (is_lengthen)
		timer_lengthen.on_update(delta);
	if (is_accelerate)
		timer_accelerate.on_update(delta);
	if (!is_die)
	{
		timer_move.on_update(delta);
		timer_time_score.on_update(delta);
	}
		
	
}

void Snake::on_draw(Canvas* canvas, Vector2 anchor)
{
	if (head_node == nullptr) return;

	// 绘制蛇头
	int i = head_node->position.x;
	int j = head_node->position.y;
	float lengthen_ring_half_width = 0.1;
	float accelerate_ring_half_width = 0.2;
	int node_idx = 1;
	int idx_sum = real_len * 2;
	char c = 'o';
	WORD color = body_color | eyes_color;

	if (check_is_die())
		c = 'x';

	if (check_is_accelerate())
	{
		float pct = node_idx * 1.0 / idx_sum;
		if (pct <= (1 - timer_accelerate.get_remaining_time_pct()) + accelerate_ring_half_width &&
			pct >= (1 - timer_accelerate.get_remaining_time_pct()) - accelerate_ring_half_width)
				color = body_accelerate_color;
	}
		
	if (check_is_lengthen())
	{
		float pct = node_idx * 1.0 / idx_sum;
		if (pct <= (1 - timer_lengthen.get_remaining_time_pct()) + lengthen_ring_half_width &&
			pct >= (1 - timer_lengthen.get_remaining_time_pct()) - lengthen_ring_half_width)
				color = body_lengthen_color | eyes_color;
	}
		

	canvas->draw_char_at(anchor.x + j * 2, anchor.y + i, c, color);
	canvas->draw_char_at(anchor.x + j * 2 + 1, anchor.y + i, c, color);
	
	// 绘制蛇身
	Node* node = head_node;
	while (node->next_node)
	{
		node_idx++;
		node = node->next_node;
		i = node->position.x;
		j = node->position.y;
		c = ' ';
		color = body_color;

		if (check_is_accelerate())
		{
			float pct = node_idx * 1.0 / idx_sum;
			if (pct <= (1 - timer_accelerate.get_remaining_time_pct()) + accelerate_ring_half_width &&
				pct >= (1 - timer_accelerate.get_remaining_time_pct()) - accelerate_ring_half_width)
				color = body_accelerate_color;
		}

		if (check_is_lengthen())
		{
			float pct = node_idx * 1.0 / idx_sum;
			if (pct <= (1 - timer_lengthen.get_remaining_time_pct()) + lengthen_ring_half_width &&
				pct >= (1 - timer_lengthen.get_remaining_time_pct()) - lengthen_ring_half_width)
				color = body_lengthen_color;
		}
		
		canvas->draw_char_at(anchor.x + j * 2, anchor.y + i, c, color);
		canvas->draw_char_at(anchor.x + j * 2 + 1, anchor.y + i, c, color);
	}
}

void Snake::on_move()
{
	if (is_die) return;

	if (is_input)
	{
		is_input = false;
		dir = input_dir;
	}

	Vector2 next_position = get_next_position();
	if (is_can_move(next_position))
	{
		if (map[next_position.x][next_position.y] == MapItemType::Score)
			eat_score(next_position);
		Node* new_head = get_new_node();
		Node* old_head = this->head_node;
		new_head->position = next_position;
		new_head->next_node = old_head;
		new_head->prev_node = nullptr;
		old_head->prev_node = new_head;
		this->head_node = new_head;
		map[new_head->position.x][new_head->position.y] = MapItemType::Snake;
		//map[old_head->position.x][old_head->position.y] = MapItemType::Snake;

		if (real_len < dst_len)
		{
			real_len++;
		}
		else
		{
			Node* old_tail = this->tail_node;
			map[old_tail->position.x][old_tail->position.y] = MapItemType::Null;
			this->tail_node = old_tail->prev_node;
			if (tail_node)
				tail_node->next_node = nullptr;
			delete_node(old_tail);
			
		}
	}
	else
	{
		is_die = true;
	}
}

bool Snake::check_is_die() const
{
	return is_die;
}

bool Snake::check_is_lengthen() const
{
	return is_lengthen;
}

bool Snake::check_is_accelerate() const
{
	return is_accelerate;
}

int Snake::get_score() const
{
	return score;
}

const WORD& Snake::get_body_color() const
{
	return body_color;
}

Direction Snake::get_dir()
{
	return dir;
}

bool Snake::is_can_move(Vector2 position)
{
	if (position.x >= HEIGHT || position.x < 0 || position.y < 0 || position.y >= WIDTH)
		return false;
	return map[position.x][position.y] == MapItemType::Null
		|| map[position.x][position.y] == MapItemType::Score;
}

Vector2 Snake::get_next_position()
{
	int dx = 0, dy = 0;
	switch (this->dir)
	{
	case Direction::Down: dx = 1;
		break;
	case Direction::Left: dy = -1;
		break;
	case Direction::Right: dy = 1;
		break;
	case Direction::Up: dx = -1;
		break;

	}
	Vector2 next_position;
	next_position.x = this->head_node->position.x + dx;
	next_position.y = this->head_node->position.y + dy;
	if (next_position.x >= HEIGHT) next_position.x = 0;
	if (next_position.x < 0) next_position.x = HEIGHT - 1;
	if (next_position.y >= WIDTH) next_position.y = 0;
	if (next_position.y < 0) next_position.y = WIDTH - 1;

	return next_position;
}

int Snake::speed_to_wait_time(float speed)
{
	int wait_time = (int)(500.f - 100.f * speed);
	if (wait_time < 0) wait_time = 0;
	return wait_time;
}

void Snake::on_lengthen(WORD score_color)
{
	dst_len++;
	is_lengthen = true;
	body_lengthen_color = score_color;
	timer_lengthen.reset();
}

void Snake::on_accelerate()
{
	if (is_accelerate) return;
	is_accelerate = true;
	timer_accelerate.reset();
	timer_move.set_wait_time(speed_to_wait_time(speed * 3));
}

void Snake::stop_accelerate()
{
	is_accelerate = false;
	timer_move.set_wait_time(speed_to_wait_time(speed));
}

void Snake::eat_score(Vector2 position)
{
	for (ScoreSpawner* score_spawner : score_spawner_list)
	{
		if (score_spawner->get_score_position() != position)
			continue;
		score += score_spawner->get_score() * speed;
		score_spawner->on_disappear();
		static int cnt = 0;
		cnt++;
		if (cnt == 1)
		{
			on_lengthen(score_spawner->get_score_in_body_color());
			cnt = 0;
		}
		break;
	}
		
}

Node* Snake::get_new_node()
{
	if (node_pool_top < 0) return nullptr;
	return node_pool[node_pool_top--];
}

void Snake::delete_node(Node* node)
{
	node_pool[++node_pool_top] = node;
}
