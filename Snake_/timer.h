#ifndef _TIMER_H_
#define _TIMER_H_

#include <functional>

class Timer {
public:
	Timer() = default;
	~Timer() = default;

	void on_update(int delta);

	void reset();
	void pause();
	void resume();
	void set_callback(std::function<void()> callback);
	void set_loop(bool flag);
	void set_wait_time(int val);
	int get_remaining_time();
	float get_remaining_time_pct();

private:
	int pass_time = 0;
	int wait_time = 0;
	bool is_pause = false;
	bool is_loop = false;
	std::function<void()> callback = nullptr;
	bool is_shotted = false;
};

#endif