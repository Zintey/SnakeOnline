#include "timer.h"

void Timer::on_update(int delta)
{
	if (!this->is_pause)
		this->pass_time += delta;
	if (this->pass_time >= this->wait_time)
	{
		this->pass_time = 0;
		if (this->is_loop || (!this->is_loop && !this->is_shotted))
		{
			this->is_shotted = 1;
			if (this->callback)
				this->callback();
		}
	}
}

void Timer::reset()
{
	this->is_shotted = 0;
	this->pass_time = 0;
}

void Timer::pause()
{
	this->is_pause = true;
}

void Timer::resume()
{
	this->is_pause = false;
}

void Timer::set_callback(std::function<void()> callback)
{
	this->callback = callback;
}

void Timer::set_loop(bool flag)
{
	this->is_loop = flag;
}

void Timer::set_wait_time(int val)
{
	this->wait_time = val;
}

int Timer::get_remaining_time()
{
	return wait_time - pass_time;
}

float Timer::get_remaining_time_pct()
{
	return get_remaining_time() * 1.f / wait_time;
}
