#ifndef _MAPINFO_H_
#define _MAPINFO_H_

constexpr int HEIGHT = 30;
constexpr int WIDTH = 30;

enum class MapItemType
{
	Null = 0,
	Wall,
	Snake,
	//SnakeHead,
	Score,
	//ScoreFlash
};


#endif