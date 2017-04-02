#ifndef SIM_CORE_CAR_SERVICE_HPP
#define SIM_CORE_CAR_SERVICE_HPP

#include "sim_core_base.hpp"
#include "sim_settings.hpp"
#include "customer.hpp"
#include "gen/empirical_int_distribution_generator.hpp"
#include "gen/triangular_distribution_generator.hpp"
#include "stat/statistic.hpp"

#include <atomic>
#include <QObject>

class sim_core_car_service : public QObject, public sim_core_base
{
	Q_OBJECT

public:
	sim_core_car_service();
	virtual ~sim_core_car_service();

	void init(uint32_t group1_workers, uint32_t group2_workers, Seed seed = std::random_device{}());

	void before_replication(uint32_t replication) override;
	void after_replication(uint32_t replication) override;
	void before_simulation() override;
	void after_simulation() override;

	void on_replication_start();
	void on_refresh();
	void on_workday_end();
	void on_customer_arrive(customer *c);
	void on_order_entry(customer *c, size_t worker_index);
	void on_car_takeover(customer *c, size_t worker_index);
	void on_park_to_workshop(customer *c, size_t worker_index);
	void on_repair_start(customer *c, size_t worker_index);
	void on_repair_finish(customer *c, size_t worker_index);
	void on_car_return_start(customer *c, size_t worker_index);
	void on_car_return_finish(customer *c, size_t worker_index);

	void set_watch_mode_active(bool active);
	void set_sim_speed(double sim_speed);
	void set_refresh_rate(double refresh_rate);

signals:
	void replication_finished(int replication, double wait_for_repair_time, double wait_in_queue_time);
	void simulation_finished(double wait_for_repair_total_time, double wait_in_queue_total_time);
	void refresh_triggered();

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
	triangular_distribution_generator<> _gen_park_from_workshop_dur;

	std::uniform_real_distribution<> _dist_car_return_dur;
	Generator _gen_car_return_dur;

	statistic _stat_wait_for_repair;
	statistic _stat_wait_in_queue;
	statistic _stat_wait_for_repair_total;
	statistic _stat_wait_in_queue_total;

	std::vector<bool> _workers_group1;
	std::vector<bool> _workers_group2;

	std::queue<customer*> _customer_queue;
	std::queue<customer*> _cars_for_repair_queue;
	std::queue<customer*> _repaired_cars_queue;

	std::atomic<bool> _watch_mode_enabled;
	std::atomic<bool> _refresh_planned;
	std::atomic<double> _refresh_rate;
	std::atomic<double> _sim_speed;
};

#endif // SIM_CORE_CAR_SERVICE_HPP
