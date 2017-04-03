#include "sim_core_car_service.hpp"

#include "log/log.hpp"

#include "pool/object_pool.hpp"
#include "sim_events_car_service.hpp"

#include <thread>

multi_object_pool<
	sim_event_base,
	sim_event_replication_start,
	sim_event_refresh,
	sim_event_workday_end,
	sim_event_customer_arrive,
	sim_event_order_entry,
	sim_event_car_takeover,
	sim_event_park_to_workshop,
	sim_event_repair_start,
	sim_event_repair_finish,
	sim_event_car_return_start,
	sim_event_car_return_finish
> EVENTS_POOL;
object_pool<customer> CUSTOMERS_POOL;

std::pair<double, double> result;

sim_core_car_service::sim_core_car_service()
	: _seed(0), _dist_customer_arrive(1.0 / TIME_BETWEEN_CUSTOMERS),
	  _gen_repair_count({{1, 1, 0.4}, {2, 2, 0.15}, {3, 3, 0.14}, {4, 4, 0.12}, {5, 5, 0.1}, {6, 6, 0.09}}),
	  _gen_repair_type({{0, 0, 0.7}, {1, 1, 0.2}, {2, 2, 0.1}}),
	  _dist_simple_repair_dur(minutes(2), minutes(20)),
	  _gen_moderate_repair_dur({{minutes(10), minutes(40), 0.1}, {minutes(41), minutes(61), 0.6}, {minutes(62), minutes(100), 0.3}}),
	  _dist_complicated_repair_dur(minutes(120), minutes(260)),
	  _dist_order_entry_dur(70.0, 310.0),
	  _dist_car_takeover_dur(80.0, 160.0),
	  _gen_park_to_workshop_dur(120.0, 240.0, 540.0),
	  _gen_park_from_workshop_dur(120.0, 240.0, 540.0),
	  _dist_car_return_dur(123.0, 257.0),
	  _watch_mode_enabled(false), _refresh_planned(false), _refresh_rate(1.0), _sim_speed(1000.0)
{
}

sim_core_car_service::~sim_core_car_service()
{
}

void sim_core_car_service::init(uint32_t group1_workers, uint32_t group2_workers, Seed seed)
{
	_seed = seed;
	LOG_INFO("Seed = %u", _seed);
	_workers_group1.assign(group1_workers, false);
	_workers_group2.assign(group2_workers, false);
	_gen_seed.seed(_seed);
	_gen_customer_arrive.seed(_gen_seed());
	_gen_repair_count.seed(_gen_seed());
	_gen_repair_type.seed(_gen_seed());
	_gen_simple_repair_dur.seed(_gen_seed());
	_gen_moderate_repair_dur.seed(_gen_seed());
	_gen_complicated_repair_dur.seed(_gen_seed());
	_gen_order_entry_dur.seed(_gen_seed());
	_gen_car_takeover_dur.seed(_gen_seed());
	_gen_park_to_workshop_dur.seed(_gen_seed());
	_gen_park_from_workshop_dur.seed(_gen_seed());
	_gen_car_return_dur.seed(_gen_seed());
}

void sim_core_car_service::single_run(uint32_t replications, double max_time, uint32_t w1, uint32_t w2)
{
	single_run_seed(replications, max_time, w1, w2, std::random_device{}());
}

void sim_core_car_service::single_run_seed(uint32_t replications, double max_time, uint32_t w1, uint32_t w2, Seed seed)
{
	init(w1, w2, seed);
	simulate(replications, max_time);
	if (_state != state::STOPPED)
		emit run_finished();
}

void sim_core_car_service::multi_run(uint32_t replications, double max_time, uint32_t w1_min, uint32_t w1_max, uint32_t w2_min, uint32_t w2_max)
{
	multi_run_seed(replications, max_time, w1_min, w1_max, w2_min, w2_max, std::random_device{}());
}

void sim_core_car_service::multi_run_seed(uint32_t replications, double max_time, uint32_t w1_min, uint32_t w1_max, uint32_t w2_min, uint32_t w2_max, Seed seed)
{
	Generator g(seed);

	std::pair<double, double> best_result(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	std::pair<uint32_t, uint32_t> best_setting(0, 0);

	for (uint32_t w1 = w1_min; w1 <= w1_max; ++w1)
	{
		for (uint32_t w2 = w2_min; w2 <= w2_max; ++w2)
		{
			init(w1, w2, g());
			simulate(replications, max_time);
			if (_state == state::STOPPED) return;
			if (_error({w1, w2}, result) < _error(best_setting, best_result))
			{
				best_result = result;
				best_setting = {w1, w2};
			}
		}
	}

	if (_state != state::STOPPED)
	{
		emit best_worker_count_found(best_setting.first, best_setting.second);
		emit run_finished();
	}
}

void sim_core_car_service::before_replication(uint32_t replication)
{
	emit replication_started(replication);

	_workers_group1.assign(_workers_group1.size(), false);
	_workers_group2.assign(_workers_group2.size(), false);

	auto time = _dist_customer_arrive(_gen_customer_arrive);
	auto cust = CUSTOMERS_POOL.construct(std::max(_cur_time + time, 0.0));
	plan_event(EVENTS_POOL.construct<sim_event_customer_arrive>(_cur_time + time, this, cust));
	plan_event(EVENTS_POOL.construct<sim_event_replication_start>(0.0, this));
	plan_event(EVENTS_POOL.construct<sim_event_workday_end>(days(1.0), this));

	if (_watch_mode_enabled && !_refresh_planned)
	{
		plan_event(EVENTS_POOL.construct<sim_event_refresh>(_cur_time, this));
		_refresh_planned = true;
	}
}

void sim_core_car_service::after_replication(uint32_t replication)
{
	_clear_all_queues();
	_refresh_planned = false;
	_stat_wait_for_repair_total.add(_stat_wait_for_repair.mean());
	_stat_wait_in_queue_total.add(_stat_wait_in_queue.mean());
	_stat_queue_len_total.add(_stat_queue_len.mean(_max_time));
	_stat_cars_for_repair_queue_len_total.add(_stat_cars_for_repair_queue_len.mean(_max_time));
	_stat_repaired_cars_queue_len_total.add(_stat_repaired_cars_queue_len.mean(_max_time));
	_stat_time_in_service_total.add(_stat_time_in_service.mean());

	replication_info info;
	info.average_customer_queue_length = _stat_queue_len_total.mean();
	info.average_cars_waiting_for_repair_queue_length = _stat_cars_for_repair_queue_len_total.mean();
	info.average_repaired_cars_queue_length = _stat_repaired_cars_queue_len_total.mean();
	info.average_wait_in_queue_duration = _stat_wait_in_queue_total.mean();
	info.average_wait_for_repair_duration = _stat_wait_for_repair_total.mean();
	info.average_time_in_service_duration = _stat_time_in_service_total.mean();
	info.wait_for_repair_90_CI = confidence_interval_90_percent(_stat_wait_for_repair_total);
	info.time_in_service_90_CI = confidence_interval_90_percent(_stat_time_in_service_total);
	info.wait_in_queue_90_CI = confidence_interval_90_percent(_stat_wait_in_queue_total);
	emit replication_finished(replication, info);

	_stat_wait_for_repair.clear();
	_stat_wait_in_queue.clear();
	_stat_queue_len.clear();
	_stat_cars_for_repair_queue_len.clear();
	_stat_repaired_cars_queue_len.clear();
	_stat_time_in_service.clear();
}

void sim_core_car_service::before_simulation()
{
	emit simulation_started(_workers_group1.size(), _workers_group2.size());

	_watch_max_time = false;
}

void sim_core_car_service::after_simulation()
{
	emit simulation_finished(_workers_group1.size(), _workers_group2.size(), _stat_wait_for_repair_total.mean(),
							 _stat_wait_in_queue_total.mean(), _stat_queue_len_total.mean(), _stat_time_in_service_total.mean());
	result = {_stat_wait_for_repair_total.mean(), _stat_wait_in_queue_total.mean()};

	_stat_wait_for_repair_total.clear();
	_stat_wait_in_queue_total.clear();
	_stat_queue_len_total.clear();
	_stat_cars_for_repair_queue_len_total.clear();
	_stat_repaired_cars_queue_len_total.clear();
	_stat_time_in_service_total.clear();
}

void sim_core_car_service::on_stopped()
{
	sim_core_base::on_stopped();
	_clear_all_queues();
	_refresh_planned = false;
	_stat_wait_for_repair.clear();
	_stat_wait_in_queue.clear();
	_stat_queue_len.clear();
	_stat_cars_for_repair_queue_len.clear();
	_stat_repaired_cars_queue_len.clear();
	_stat_time_in_service.clear();
	_stat_wait_for_repair_total.clear();
	_stat_wait_in_queue_total.clear();
	_stat_queue_len_total.clear();
	_stat_cars_for_repair_queue_len_total.clear();
	_stat_repaired_cars_queue_len_total.clear();
	_stat_time_in_service_total.clear();
}

void sim_core_car_service::on_replication_start()
{
	if (_customer_queue.empty())
		return;

	for (size_t i = 0; i < _workers_group1.size(); ++i)
	{
		if (_workers_group1[i]) continue;
		if (_customer_queue.empty()) break;

		_workers_group1[i] = true;
		auto cust = _customer_queue.front();
		_customer_queue.pop();
		_stat_queue_len.add(_customer_queue.size(), _cur_time);
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, cust, i));
	}
}

void sim_core_car_service::on_refresh()
{
	refresh_info info;
	info.workers_1_working = std::count(_workers_group1.begin(), _workers_group1.end(), true);
	info.workers_2_working = std::count(_workers_group2.begin(), _workers_group2.end(), true);
	info.customer_queue_length = _customer_queue.size();
	info.cars_waiting_for_repair_queue_length = _cars_for_repair_queue.size();
	info.repaired_cars_queue_length = _repaired_cars_queue.size();
	info.average_customer_queue_length = _stat_queue_len.mean(std::min(_cur_time, _max_time));
	info.average_cars_waiting_for_repair_queue_length = _stat_cars_for_repair_queue_len.mean(std::min(_cur_time, _max_time));
	info.average_repaired_cars_queue_length = _stat_repaired_cars_queue_len.mean(std::min(_cur_time, _max_time));
	info.average_wait_in_queue_duration = _stat_wait_in_queue.mean();
	info.average_wait_for_repair_duration = _stat_wait_for_repair.mean();
	info.average_time_in_service_duration = _stat_time_in_service.mean();

	emit refresh_triggered(_cur_time, info);
	std::this_thread::sleep_for(std::chrono::milliseconds(_sim_speed));

	if (!_watch_mode_enabled)
	{
		_refresh_planned = false;
		return;
	}

	if (is_event_queue_empty())
		return;

	plan_event(EVENTS_POOL.construct<sim_event_refresh>(_cur_time + _refresh_rate, this));
}

void sim_core_car_service::on_workday_end()
{
	_clear_queue(_customer_queue);
	_stat_queue_len.add(_customer_queue.size(), _cur_time);
	if (_cur_time + days(1.0) > _max_time)
		return;
	plan_event(EVENTS_POOL.construct<sim_event_workday_end>(_cur_time + days(1.0), this));
}

void sim_core_car_service::on_customer_arrive(customer *c)
{
	auto time = _dist_customer_arrive(_gen_customer_arrive);
	if (_cur_time + time <= _max_time)
	{
		auto cust = CUSTOMERS_POOL.construct(std::max(_cur_time + time, 0.0));
		plan_event(EVENTS_POOL.construct<sim_event_customer_arrive>(_cur_time + time, this, cust));
	}

	c->set_queue_wait_start(std::max(_cur_time, 0.0));

	if (_cur_time < 0.0)
	{
		_customer_queue.push(c);
		_stat_queue_len.add(_customer_queue.size(), 0.0);
		return;
	}

	for (size_t i = 0; i < _workers_group1.size(); ++i)
	{
		if (_workers_group1[i]) continue; // if working

		_workers_group1[i] = true;
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, c, i));
		return;
	}

	_customer_queue.push(c);
	if (_cur_time <= _max_time)
		_stat_queue_len.add(_customer_queue.size(), _cur_time);
}

void sim_core_car_service::on_order_entry(customer *c, size_t worker_index)
{
	_stat_wait_in_queue.add(_cur_time - c->get_queue_wait_start());

	auto duration = _dist_order_entry_dur(_gen_order_entry_dur);
	auto repair_dur = _generate_repair_duration();
	c->set_car_repair_duration(repair_dur);
	plan_event(EVENTS_POOL.construct<sim_event_car_takeover>(_cur_time + duration, this, c, worker_index));
}

void sim_core_car_service::on_car_takeover(customer *c, size_t worker_index)
{
	auto takeover_dur = _dist_car_takeover_dur(_gen_car_takeover_dur);
	auto park_to_workshop_dur = _gen_park_to_workshop_dur();
	plan_event(EVENTS_POOL.construct<sim_event_park_to_workshop>(_cur_time + takeover_dur + park_to_workshop_dur, this, c, worker_index));

	c->set_repair_wait_start(_cur_time + takeover_dur);
}

void sim_core_car_service::on_park_to_workshop(customer *c, size_t worker_index)
{
	// group 2 worker
	bool found = false;
	for (size_t i = 0; i < _workers_group2.size(); ++i)
	{
		if (_workers_group2[i]) continue;

		found = true;
		_workers_group2[i] = true;
		plan_event(EVENTS_POOL.construct<sim_event_repair_start>(_cur_time, this, c, i));
		break;
	}
	if (!found)
	{
		_cars_for_repair_queue.push(c);
		if (_cur_time <= _max_time)
			_stat_cars_for_repair_queue_len.add(_cars_for_repair_queue.size(), _cur_time);
	}

	// group 1 worker
	if (_repaired_cars_queue.empty())
	{
		if (_customer_queue.empty())
		{
			_workers_group1[worker_index] = false;
			return;
		}

		auto cust = _customer_queue.front();
		_customer_queue.pop();
		if (_cur_time <= _max_time)
			_stat_queue_len.add(_customer_queue.size(), _cur_time);
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, cust, worker_index));
		return;
	}

	auto cust = _repaired_cars_queue.front();
	_repaired_cars_queue.pop();
	if (_cur_time <= _max_time)
		_stat_repaired_cars_queue_len.add(_repaired_cars_queue.size(), _cur_time);
	plan_event(EVENTS_POOL.construct<sim_event_car_return_start>(_cur_time + _gen_park_from_workshop_dur(), this, cust, worker_index));
}

void sim_core_car_service::on_repair_start(customer *c, size_t worker_index)
{
	plan_event(EVENTS_POOL.construct<sim_event_repair_finish>(_cur_time + c->get_car_repair_duration(), this, c, worker_index));
}

void sim_core_car_service::on_repair_finish(customer *c, size_t worker_index)
{
	bool found = false;
	for (size_t i = 0; i < _workers_group1.size(); ++i)
	{
		if (_workers_group1[i]) continue;

		found = true;
		_workers_group1[i] = true;
		plan_event(EVENTS_POOL.construct<sim_event_car_return_start>(_cur_time + _gen_park_from_workshop_dur(), this, c, i));
		break;
	}
	if (!found)
	{
		_repaired_cars_queue.push(c);
		if (_cur_time <= _max_time)
			_stat_repaired_cars_queue_len.add(_repaired_cars_queue.size(), _cur_time);
	}

	if (_cars_for_repair_queue.empty())
	{
		_workers_group2[worker_index] = false;
		return;
	}

	auto cust = _cars_for_repair_queue.front();
	_cars_for_repair_queue.pop();
	if (_cur_time <= _max_time)
		_stat_cars_for_repair_queue_len.add(_cars_for_repair_queue.size(), _cur_time);
	plan_event(EVENTS_POOL.construct<sim_event_repair_start>(_cur_time, this, cust, worker_index));
}

void sim_core_car_service::on_car_return_start(customer *c, size_t worker_index)
{
	_stat_wait_for_repair.add(_cur_time - c->get_repair_wait_start());

	auto duration = _dist_car_return_dur(_gen_car_return_dur);
	plan_event(EVENTS_POOL.construct<sim_event_car_return_finish>(_cur_time + duration, this, c, worker_index));
}

void sim_core_car_service::on_car_return_finish(customer *c, size_t worker_index)
{
	_stat_time_in_service.add(_cur_time - c->arrive_time);
	CUSTOMERS_POOL.destroy(c);

	if (_repaired_cars_queue.empty())
	{
		if (_customer_queue.empty())
		{
			_workers_group1[worker_index] = false;
			return;
		}

		auto cust = _customer_queue.front();
		_customer_queue.pop();
		if (_cur_time <= _max_time)
			_stat_queue_len.add(_customer_queue.size(), _cur_time);
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, cust, worker_index));
		return;
	}

	auto cust = _repaired_cars_queue.front();
	_repaired_cars_queue.pop();
	if (_cur_time <= _max_time)
		_stat_repaired_cars_queue_len.add(_repaired_cars_queue.size(), _cur_time);
	plan_event(EVENTS_POOL.construct<sim_event_car_return_start>(_cur_time + _gen_park_from_workshop_dur(), this, cust, worker_index));
}

void sim_core_car_service::set_watch_mode_active(bool active)
{
	_watch_mode_enabled = active;

	if (_state != state::RUNNING && _state != state::PAUSED)
	{
		_refresh_planned = false;
		return;
	}

	if (active && !_refresh_planned)
	{
		plan_event(EVENTS_POOL.construct<sim_event_refresh>(_cur_time + _refresh_rate, this));
		_refresh_planned = true;
	}
}

void sim_core_car_service::set_sim_speed(double sim_speed)
{
	_sim_speed = sim_speed;
}

void sim_core_car_service::set_refresh_rate(double refresh_rate)
{
	_refresh_rate = refresh_rate;
}

Seed sim_core_car_service::get_seed() const
{
	return _seed;
}

void sim_core_car_service::_init_time()
{
	_cur_time = -hours(HEAT_UP_HOURS);
}

void sim_core_car_service::_delete_event(sim_event_base *event) const
{
	EVENTS_POOL.destroy(event);
}

void sim_core_car_service::_clear_queue(std::queue<customer *> &queue)
{
	while (!queue.empty())
	{
		auto c = queue.front();
		queue.pop();
		CUSTOMERS_POOL.destroy(c);
	}
}

void sim_core_car_service::_clear_all_queues()
{
	_clear_queue(_customer_queue);
	_clear_queue(_cars_for_repair_queue);
	_clear_queue(_repaired_cars_queue);
}

double sim_core_car_service::_generate_repair_duration()
{
	double ret = 0.0;
	int repair_count = _gen_repair_count();
	for (int i = 0; i < repair_count; ++i)
	{
		auto repair_type = _gen_repair_type();
		switch (repair_type)
		{
			case 0:
				ret += _dist_simple_repair_dur(_gen_simple_repair_dur);
				break;
			case 1:
				ret += _gen_moderate_repair_dur();
				break;
			case 2:
				ret += _dist_complicated_repair_dur(_gen_complicated_repair_dur);
				break;
			default:
				break;
		}
	}
	return ret;
}

double sim_core_car_service::_error(const std::pair<uint32_t, uint32_t> &setting, const std::pair<double, double> &result)
{
	if (result.first > hours(5.0)) return std::numeric_limits<double>::max();
	if (result.second > minutes(3.0)) return std::numeric_limits<double>::max();
	return setting.first + setting.second;
}
