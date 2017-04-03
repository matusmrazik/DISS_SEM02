#include "sim_core_base.hpp"

#include <thread>

sim_core_base::sim_core_base()
	: _cur_time(0.0), _max_time(0.0), _state(state::READY), _watch_max_time(true)
{
}

sim_core_base::~sim_core_base()
{
}

void sim_core_base::simulate(uint32_t replications, double max_time)
{
	_max_time = max_time;
	_state = state::RUNNING;
	before_simulation();
	for (uint32_t repl = 1; repl <= replications; ++repl)
	{
		_init_time();
		before_replication(repl);
		while (!_event_queue.empty())
		{
			if (_state == state::PAUSED)
				on_paused();
			if (_state == state::STOPPED)
			{
				on_stopped();
				return;
			}
			if (!_process_next_event())
				break;
		}
		_clear_event_queue();
		after_replication(repl);
	}
	after_simulation();
	_state = state::FINISHED;
}

void sim_core_base::plan_event(sim_event_base *event)
{
	_event_queue.push(event);
}

void sim_core_base::pause_resume()
{
	if (_state == state::RUNNING)
		_state = state::PAUSED;
	else if (_state == state::PAUSED)
		_state = state::RUNNING;
}

void sim_core_base::stop()
{
	_state = state::STOPPED;
}

void sim_core_base::on_paused()
{
	while (_state == state::PAUSED)
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void sim_core_base::on_stopped()
{
	_clear_event_queue();
}

double sim_core_base::get_cur_time() const
{
	return _cur_time;
}

sim_core_base::state sim_core_base::get_state() const
{
	return _state;
}

bool sim_core_base::is_event_queue_empty() const
{
	return _event_queue.empty();
}

bool sim_core_base::_process_next_event()
{
	auto event = _event_queue.top_pop();
	_cur_time = event->time;
	if (_watch_max_time && (_cur_time > _max_time))
	{
		_delete_event(event);
		return false;
	}
	event->execute();
	_delete_event(event);
	return true;
}

void sim_core_base::_clear_event_queue()
{
	while (!_event_queue.empty())
	{
		auto event = _event_queue.top_pop();
		_delete_event(event);
	}
}

void sim_core_base::_init_time()
{
	_cur_time = 0.0;
}

void sim_core_base::_delete_event(sim_event_base *event) const
{
	delete event;
}
