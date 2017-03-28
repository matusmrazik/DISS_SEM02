#include "sim_events_car_service.hpp"

sim_event_car_service::sim_event_car_service(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_base(time),
	  _sim(sim), _customer(c), _worker(worker)
{
}

sim_event_car_service::~sim_event_car_service()
{
}

sim_event_replication_start::sim_event_replication_start(double time, sim_core_car_service *sim)
	: sim_event_base(time), _sim(sim)
{
}

sim_event_replication_start::~sim_event_replication_start()
{
}

void sim_event_replication_start::execute()
{
	_sim->on_replication_start();
}

sim_event_customer_arrive::sim_event_customer_arrive(double time, sim_core_car_service *sim, customer *c)
	: sim_event_car_service(time, sim, c)
{
}

sim_event_customer_arrive::~sim_event_customer_arrive()
{
}

void sim_event_customer_arrive::execute()
{
	_sim->on_customer_arrive(_customer);
}

sim_event_order_entry::sim_event_order_entry(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_order_entry::~sim_event_order_entry()
{
}

void sim_event_order_entry::execute()
{
	_sim->on_order_entry(_customer, _worker);
}

sim_event_car_takeover::sim_event_car_takeover(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_car_takeover::~sim_event_car_takeover()
{
}

void sim_event_car_takeover::execute()
{
	_sim->on_car_takeover(_customer, _worker);
}

sim_event_park_to_workshop::sim_event_park_to_workshop(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_park_to_workshop::~sim_event_park_to_workshop()
{
}

void sim_event_park_to_workshop::execute()
{
	_sim->on_park_to_workshop(_customer, _worker);
}

sim_event_repair_start::sim_event_repair_start(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_repair_start::~sim_event_repair_start()
{
}

void sim_event_repair_start::execute()
{
	_sim->on_repair_start(_customer, _worker);
}

sim_event_repair_finish::sim_event_repair_finish(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_repair_finish::~sim_event_repair_finish()
{
}

void sim_event_repair_finish::execute()
{
	_sim->on_repair_finish(_customer, _worker);
}

sim_event_car_return_start::sim_event_car_return_start(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_car_return_start::~sim_event_car_return_start()
{
}

void sim_event_car_return_start::execute()
{
	_sim->on_car_return_start(_customer, _worker);
}

sim_event_car_return_finish::sim_event_car_return_finish(double time, sim_core_car_service *sim, customer *c, size_t worker)
	: sim_event_car_service(time, sim, c, worker)
{
}

sim_event_car_return_finish::~sim_event_car_return_finish()
{
}

void sim_event_car_return_finish::execute()
{
	_sim->on_car_return_finish(_customer, _worker);
}

sim_event_workday_end::sim_event_workday_end(double time, sim_core_car_service *sim)
	: sim_event_car_service(time, sim, nullptr)
{
}

sim_event_workday_end::~sim_event_workday_end()
{
}

void sim_event_workday_end::execute()
{
	_sim->on_workday_end();
}
