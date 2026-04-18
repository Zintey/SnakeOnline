#ifndef _SCORE_SPAWNER_H_
#define _SCORE_SPAWNER_H_

#include "mapinfo.h"
#include "timer.h"
#include "vector2.h"
#include "canvas.h"

extern MapItemType map[HEIGHT][WIDTH];

class ScoreSpawner
{
public:
	ScoreSpawner(float score, int spawn_delay_time, int valid_time,
		WORD score_color, WORD score_in_body_color, Vector2 pct_anchor);
	~ScoreSpawner() = default;

	void on_update(int delta);

	void on_draw(Canvas* canvas, Vector2 anchor);

	void on_spawn();

	void on_disappear();

	float get_score() const;

	bool check_is_flash() const;

	float get_remain_vaild_time_pct();

	bool check_is_spawned() const;

	const Vector2& get_score_position() const;

	const WORD& get_score_in_body_color();

private:
	int spawn_delay_time = 4000;
	int valid_time = 6000;
	float score = 0;
	Timer timer_wait;
	Timer timer_spawn;
	bool is_spawned = false;
	Vector2 score_position = { 0 };
	Vector2 pct_anchor = { 0, 0 };

	Timer timer_score_flash;
	bool is_flash = false;

	WORD score_color = BG_GREEN;
	WORD score_in_body_color = BG_DARK_GREEN;
private:
	Vector2 get_random_position();
};

#endif