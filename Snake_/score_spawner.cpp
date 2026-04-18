#include "score_spawner.h"

ScoreSpawner::ScoreSpawner(float score, int spawn_delay_time, int valid_time,
	WORD score_color, WORD score_in_body_color, Vector2 pct_anchor)
	:score(score), spawn_delay_time(spawn_delay_time), valid_time(valid_time),
	score_color(score_color), score_in_body_color(score_in_body_color), pct_anchor(pct_anchor)
{
	timer_spawn.set_loop(false);
	timer_spawn.set_wait_time(spawn_delay_time);
	timer_spawn.set_callback([&]() {
		on_spawn();
		});

	timer_wait.set_loop(false);
	timer_wait.set_wait_time(valid_time);
	timer_wait.set_callback([&]() {
		on_disappear();
		});

	timer_score_flash.set_loop(true);
	timer_score_flash.set_wait_time(400);
	timer_score_flash.set_callback([&]() {
		is_flash = !is_flash;
		});
}

void ScoreSpawner::on_update(int delta)
{
	if (is_spawned)
	{
		timer_wait.on_update(delta);
		timer_score_flash.on_update(delta);
	}
	if (!is_spawned)
		timer_spawn.on_update(delta);
		
}

void ScoreSpawner::on_draw(Canvas* canvas, Vector2 anchor)
{
	if (!check_is_spawned()) return;

	int i = score_position.x;
	int j = score_position.y;
	char c = ' ';
	WORD color = score_color;
	if (check_is_flash())
		color = BG_DARK_GRAY;
	canvas->draw_char_at(anchor.x + j * 2, anchor.y + i, c, color);
	canvas->draw_char_at(anchor.x + j * 2 + 1, anchor.y + i, c, color);

	int len = HEIGHT;
	for (int j = 0; j < len; j++)
	{
		float pct = j * 1.0 / len;
		WORD color = BG_BLACK;
		if (pct <= get_remain_vaild_time_pct())
			color = score_color;
		canvas->draw_char_at(pct_anchor.x, pct_anchor.y + j, ' ', color);
		canvas->draw_char_at(pct_anchor.x + 1, pct_anchor.y + j, ' ', color);
	}
}

void ScoreSpawner::on_spawn()
{
	if (is_spawned) return;
	is_spawned = true;
	is_flash = false;
	timer_wait.reset();
	timer_score_flash.reset();

	score_position = get_random_position();
	map[score_position.x][score_position.y] = MapItemType::Score;
}

void ScoreSpawner::on_disappear()
{
	if (!is_spawned) return;
	is_spawned = false;
	timer_spawn.reset();
	map[score_position.x][score_position.y] = MapItemType::Null;
	score_position.x = -1;
	score_position.y = -1;
}

float ScoreSpawner::get_score() const
{
	return score;
}

bool ScoreSpawner::check_is_flash() const
{
	return is_flash;
}

float ScoreSpawner::get_remain_vaild_time_pct()
{
	return timer_wait.get_remaining_time_pct();
}

bool ScoreSpawner::check_is_spawned() const
{
	return is_spawned;
}

const Vector2& ScoreSpawner::get_score_position() const
{
	return score_position;
}

const WORD& ScoreSpawner::get_score_in_body_color()
{
	return score_in_body_color;
}

Vector2 ScoreSpawner::get_random_position()
{ 
	int x = rand() % HEIGHT;
	int y = rand() % WIDTH;
	while (map[x][y] != MapItemType::Null)
		x = rand() % HEIGHT, y = rand() % WIDTH;
	return Vector2{ x, y };
}
