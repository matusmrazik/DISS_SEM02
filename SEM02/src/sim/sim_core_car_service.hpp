#ifndef SIM_CORE_CAR_SERVICE_HPP
#define SIM_CORE_CAR_SERVICE_HPP

#include "sim_core_base.hpp"
#include "sim_settings.hpp"
#include "customer.hpp"
#include "gen/empirical_int_distribution_generator.hpp"
#include "gen/triangular_distribution_generator.hpp"
#include "stat/statistic_discrete.hpp"

class sim_core_car_service : public sim_core_base
{
public:
	sim_core_car_service();
	virtual ~sim_core_car_service();

	void init(uint32_t group1_workers, uint32_t group2_workers, Seed seed = std::random_device{}());

	void before_replication(uint32_t replication) override;
	void after_replication(uint32_t replication) override;
	void before_simulation() override;
	void after_simulation() override;

	void on_replication_start();
	void on_customer_arrive(customer *c);
	void on_order_entry(customer *c, size_t worker_index);
	void on_car_takeover(customer *c, size_t worker_index);
	void on_park_to_workshop(customer *c, size_t worker_index);
	void on_repair_start(customer *c, size_t worker_index);
	void on_repair_finish(customer *c, size_t worker_index);
	void on_car_return_start(customer *c, size_t worker_index);
	void on_car_return_finish(customer *c, size_t worker_index);
	void on_workday_end();

private:
	void _init_time() override;
	void _delete_event(sim_event_base *event) const override;
	void _clear_queue(std::queue<customer*> &queue);
	void _clear_all_queues();
	double _generate_repair_duration();

private:
	Seed _seed;

	Generator _gen_seed;

	Generator _gen_customer_arrive;
	std::exponential_distribution<> _dist_customer_arrive;

	empirical_int_distribution_generator<> _gen_repair_count;
	empirical_int_distribution_generator<> _gen_repair_type;

	Generator _gen_simple_repair_dur;
	std::uniform_int_distribution<> _dist_simple_repair_dur;
	empirical_int_distribution_generator<> _gen_moderate_repair_dur;
	Generator _gen_complicated_repair_dur;
	std::uniform_int_distribution<> _dist_complicated_repair_dur;

	std::uniform_real_distribution<> _dist_order_entry_dur;
	Generator _gen_order_entry_dur;

	std::uniform_real_distribution<> _dist_car_takeover_dur;
	Generator _gen_car_takeover_dur;

	triangular_distribution_generator<> _gen_park_to_workshop_dur;

	std::uniform_real_distribution<> _dist_car_return_dur;
	Generator _gen_car_return_dur;

	statistic_discrete _stat_wait_for_repair;
	statistic_discrete _stat_wait_in_queue;
	statistic_discrete _stat_wait_for_repair_total;
	statistic_discrete _stat_wait_in_queue_total;

	std::vector<bool> _workers_group1;
	std::vector<bool> _workers_group2;

	std::queue<customer*> _customer_queue;
	std::queue<customer*> _cars_for_repair_queue;
	std::queue<customer*> _repaired_cars_queue;
};

#endif // SIM_CORE_CAR_SERVICE_HPP
