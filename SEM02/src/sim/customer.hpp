#ifndef CUSTOMER_HPP
#define CUSTOMER_HPP

class customer
{
public:
	customer(double arrive_time);
	~customer();

	void set_car_repair_duration(const double duration);
	void set_queue_wait_start(const double time);
	void set_repair_wait_start(const double time);

	double get_car_repair_duration() const;
	double get_queue_wait_start() const;
	double get_repair_wait_start() const;

public:
	const double arrive_time;

private:
	double _car_repair_duration;
	double _queue_wait_start;
	double _repair_wait_start;
};

#endif // CUSTOMER_HPP
