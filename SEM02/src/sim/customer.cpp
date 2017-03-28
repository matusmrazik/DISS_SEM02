#include "customer.hpp"

customer::customer(double arrive_time)
	: arrive_time(arrive_time), _car_repair_duration(0.0)
{
}

customer::~customer()
{
}

void customer::set_car_repair_duration(const double duration)
{
	_car_repair_duration = duration;
}

void customer::set_queue_wait_start(const double time)
{
	_queue_wait_start = time;
}

void customer::set_repair_wait_start(const double time)
{
	_repair_wait_start = time;
}

double customer::get_car_repair_duration() const
{
	return _car_repair_duration;
}

double customer::get_queue_wait_start() const
{
	return _queue_wait_start;
}

double customer::get_repair_wait_start() const
{
	return _repair_wait_start;
}
