#include "sim_core_car_service.hpp"

#include "log/log.hpp"

#include "pool/object_pool.hpp"
#include "sim_events_car_service.hpp"

multi_object_pool<
	sim_event_base,
	sim_event_replication_start,
	sim_event_customer_arrive,
	sim_event_order_entry,
	sim_event_car_takeover,
	sim_event_park_to_workshop,
	sim_event_repair_start,
	sim_event_repair_finish,
	sim_event_car_return_start,
	sim_event_car_return_finish,
	sim_event_workday_end
> EVENTS_POOL;
object_pool<customer> CUSTOMERS_POOL;

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
	  _dist_car_return_dur(123.0, 257.0)
{
/*	double avg = 0.0;
	int n = 1000000000;
	init(0, 0);
	for (int i = 1; i <= n; ++i)
	{
		if (i % 1000000 == 0)
			printf("%d\n", i);
		switch (_gen_repair_type())
		{
			case 0:
				avg += _dist_simple_repair_dur(_gen_simple_repair_dur);
				break;
			case 1:
				avg += _gen_moderate_repair_dur();
				break;
			case 2:
				avg += _dist_complicated_repair_dur(_gen_complicated_repair_dur);
				break;
			default:
				break;
		}
	}
	avg /= static_cast<double>(n);
	printf("%f\n", avg);*/
}

sim_core_car_service::~sim_core_car_service()
{
}

void sim_core_car_service::init(uint32_t group1_workers, uint32_t group2_workers, Seed seed)
{
	_seed = seed;
	LOG_INFO("Seed = %u", _seed);
	// params
	_workers_group1.clear();
	_workers_group1.resize(group1_workers, false);
	_workers_group2.clear();
	_workers_group2.resize(group2_workers, false);
	// end params
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
	_gen_car_return_dur.seed(_gen_seed());
}

void sim_core_car_service::before_replication(uint32_t)
{
	_workers_group1.assign(_workers_group1.size(), false);
	_workers_group2.assign(_workers_group2.size(), false);

	auto time = _dist_customer_arrive(_gen_customer_arrive);
	auto cust = CUSTOMERS_POOL.construct(_cur_time + time);
	plan_event(EVENTS_POOL.construct<sim_event_customer_arrive>(cust->arrive_time, this, cust));
	plan_event(EVENTS_POOL.construct<sim_event_replication_start>(0.0, this));
	plan_event(EVENTS_POOL.construct<sim_event_workday_end>(days(1.0), this));
}

void sim_core_car_service::after_replication(uint32_t)
{
	_clear_all_queues();
	_stat_wait_for_repair_total.add(_stat_wait_for_repair.mean());
	_stat_wait_in_queue_total.add(_stat_wait_in_queue.mean());
	_stat_wait_for_repair.clear();
	_stat_wait_in_queue.clear();
}

void sim_core_car_service::before_simulation()
{
	_watch_max_time = false;
}

void sim_core_car_service::after_simulation()
{
	printf("Priemerny cas cakania na opravu: %f s\n", _stat_wait_for_repair_total.mean());
	printf("Priemerny cas cakania v rade: %f s\n", _stat_wait_in_queue_total.mean());
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
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, cust, i));
	}
}

void sim_core_car_service::on_customer_arrive(customer *c)
{
	auto time = _dist_customer_arrive(_gen_customer_arrive);
	if (_cur_time + time <= _max_time)
	{
		auto cust = CUSTOMERS_POOL.construct(_cur_time + time);
		plan_event(EVENTS_POOL.construct<sim_event_customer_arrive>(cust->arrive_time, this, cust));
	}

	c->set_queue_wait_start(_cur_time);

	if (_cur_time < 0.0)
	{
		_customer_queue.push(c);
		return;
	}

	for (size_t i = 0; i < _workers_group1.size(); ++i)
	{
		if (_workers_group1[i]) // if working
			continue;

		_workers_group1[i] = true;
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, c, i));
		return;
	}

	_customer_queue.push(c);
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
	}
	if (!found)
		_cars_for_repair_queue.push(c);

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
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, cust, worker_index));
		return;
	}

	auto cust = _repaired_cars_queue.front();
	_repaired_cars_queue.pop();
	plan_event(EVENTS_POOL.construct<sim_event_car_return_start>(_cur_time, this, cust, worker_index));
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
		plan_event(EVENTS_POOL.construct<sim_event_car_return_start>(_cur_time, this, c, i));
		break;
	}
	if (!found)
		_repaired_cars_queue.push(c);

	if (_cars_for_repair_queue.empty())
	{
		_workers_group2[worker_index] = false;
		return;
	}

	auto cust = _cars_for_repair_queue.front();
	_cars_for_repair_queue.pop();
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
		plan_event(EVENTS_POOL.construct<sim_event_order_entry>(_cur_time, this, cust, worker_index));
		return;
	}

	auto cust = _repaired_cars_queue.front();
	_repaired_cars_queue.pop();
	plan_event(EVENTS_POOL.construct<sim_event_car_return_start>(_cur_time, this, cust, worker_index));
}

void sim_core_car_service::on_workday_end()
{
	_clear_queue(_customer_queue);
	if (_cur_time + days(1.0) > _max_time)
		return;
	plan_event(EVENTS_POOL.construct<sim_event_workday_end>(_cur_time + days(1.0), this));
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
		switch (_gen_repair_type())
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
