#include "wall_spawner.h"

extern MapItemType map[HEIGHT][WIDTH];

WallSpawner::WallSpawner(int spawn_delay_time)
	:spawn_delay_time(spawn_delay_time)
{
	timer_spawn.set_loop(true);
	timer_spawn.set_wait_time(spawn_delay_time);
	timer_spawn.set_callback([&]() {
		on_spawn();
		});
}

void WallSpawner::on_update(int delta)
{
	timer_spawn.on_update(delta);
}

void WallSpawner::on_spawn()
{
	Vector2 spawn_position = get_random_position();
	map[spawn_position.x][spawn_position.y] = MapItemType::Wall;
}

Vector2 WallSpawner::get_random_position()
{
	int x = rand() % HEIGHT;
	int y = rand() % WIDTH;
	while (map[x][y] != MapItemType::Null)
		x = rand() % HEIGHT, y = rand() % WIDTH;
	return Vector2{ x, y };
}
