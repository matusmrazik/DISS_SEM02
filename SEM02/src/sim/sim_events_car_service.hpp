#ifndef SIM_EVENTS_CAR_SERVICE_HPP
#define SIM_EVENTS_CAR_SERVICE_HPP

#include "sim_event_base.hpp"
#include "sim_core_car_service.hpp"
#include "customer.hpp"

class sim_event_car_service : public sim_event_base
{
public:
	sim_event_car_service(double time, sim_core_car_service *sim, customer *c, size_t worker = static_cast<size_t>(-1));
	virtual ~sim_event_car_service();

protected:
	sim_core_car_service *_sim;
	customer *_customer;
	size_t _worker;
};

class sim_event_replication_start : public sim_event_base
{
public:
	sim_event_replication_start(double time, sim_core_car_service *sim);
	~sim_event_replication_start();

	void execute() override;

protected:
	sim_core_car_service *_sim;
};

class sim_event_refresh : public sim_event_base
{
public:
	sim_event_refresh(double time, sim_core_car_service *sim);
	~sim_event_refresh();

	void execute() override;

protected:
	sim_core_car_service *_sim;
};

class sim_event_workday_end : public sim_event_base
{
public:
	sim_event_workday_end(double time, sim_core_car_service *sim);
	~sim_event_workday_end();

	void execute() override;

protected:
	sim_core_car_service *_sim;
};

class sim_event_customer_arrive : public sim_event_car_service
{
public:
	sim_event_customer_arrive(double time, sim_core_car_service *sim, customer *c);
	~sim_event_customer_arrive();

	void execute() override;
};

class sim_event_order_entry : public sim_event_car_service
{
public:
	sim_event_order_entry(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_order_entry();

	void execute() override;
};

class sim_event_car_takeover : public sim_event_car_service
{
public:
	sim_event_car_takeover(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_car_takeover();

	void execute() override;
};

class sim_event_park_to_workshop : public sim_event_car_service
{
public:
	sim_event_park_to_workshop(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_park_to_workshop();

	void execute() override;
};

class sim_event_repair_start : public sim_event_car_service
{
public:
	sim_event_repair_start(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_repair_start();

	void execute() override;
};

class sim_event_repair_finish : public sim_event_car_service
{
public:
	sim_event_repair_finish(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_repair_finish();

	void execute() override;
};

class sim_event_car_return_start : public sim_event_car_service
{
public:
	sim_event_car_return_start(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_car_return_start();

	void execute() override;
};

class sim_event_car_return_finish : public sim_event_car_service
{
public:
	sim_event_car_return_finish(double time, sim_core_car_service *sim, customer *c, size_t worker);
	~sim_event_car_return_finish();

	void execute() override;
};

#endif // SIM_EVENTS_CAR_SERVICE_HPP
