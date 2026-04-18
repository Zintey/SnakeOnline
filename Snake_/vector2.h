#ifndef _VECTOR_2_H
#define _VECTOR_2_H

struct Vector2 {
	int x;
	int y;
	friend bool operator== (const Vector2& vec1, const Vector2& vec2)
	{
		return vec1.x == vec2.x && vec1.y == vec2.y;
	}
	friend bool operator!= (const Vector2& vec1, const Vector2& vec2)
	{
		return !(vec1.x == vec2.x && vec1.y == vec2.y);
	}
};

#endif