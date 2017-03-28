#ifndef SIM_EVENT_BASE_HPP
#define SIM_EVENT_BASE_HPP

class sim_event_base
{
public:
	sim_event_base(double time);
	virtual ~sim_event_base();

	virtual void execute() = 0;

public:
	const double time;
};

struct sim_event_comp
{
	inline bool operator()(const sim_event_base *left, const sim_event_base *right) const noexcept
	{
		return left->time > right->time;
	}
};

#endif // SIM_EVENT_BASE_HPP
