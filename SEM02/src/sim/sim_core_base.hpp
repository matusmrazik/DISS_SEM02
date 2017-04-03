#ifndef SIM_CORE_BASE_HPP
#define SIM_CORE_BASE_HPP

#include "sim_event_base.hpp"
#include "priority_queue.hpp"

#include <cinttypes>

class sim_core_base
{
public:
	enum class state
	{
		READY, RUNNING, PAUSED, STOPPED, FINISHED
	};

	sim_core_base();
	virtual ~sim_core_base();

	void simulate(uint32_t replications, double max_time);

	virtual void before_replication(uint32_t replication) = 0;
	virtual void after_replication(uint32_t replication) = 0;
	virtual void before_simulation() = 0;
	virtual void after_simulation() = 0;

	void plan_event(sim_event_base *event);

	void pause_resume();
	void stop();

	virtual void on_paused();  // called when paused by user
	virtual void on_stopped(); // called when stopped by user

	double get_cur_time() const;
	state get_state() const;
	bool is_event_queue_empty() const;

protected:
	bool _process_next_event();
	void _clear_event_queue();
	virtual void _init_time();
	virtual void _delete_event(sim_event_base *event) const;

protected:
	double _cur_time, _max_time;
	state _state;
	bool _watch_max_time;

private:
	priority_queue<sim_event_base*, std::vector<sim_event_base*>, sim_event_comp> _event_queue;
};

#endif // SIM_CORE_BASE_HPP
