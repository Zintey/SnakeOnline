#ifndef _WALL_SPAWNER_H_
#define _WALL_SPAWNER_H_

#include "timer.h"
#include "canvas.h"
#include "vector2.h"
#include "mapinfo.h"



class WallSpawner
{
public:
	WallSpawner(int spawn_delay_time);
	~WallSpawner() = default;

	void on_update(int delta);

private:
	void on_spawn();
	Vector2 get_random_position();

private:
	int spawn_delay_time = 10000;
	Timer timer_spawn;

};

#endif