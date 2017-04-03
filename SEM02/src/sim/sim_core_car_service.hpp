#ifndef SIM_CORE_CAR_SERVICE_HPP
#define SIM_CORE_CAR_SERVICE_HPP

#include "sim_core_base.hpp"
#include "sim_settings.hpp"
#include "customer.hpp"
#include "gen/empirical_int_distribution_generator.hpp"
#include "gen/triangular_distribution_generator.hpp"
#include "stat/statistic.hpp"
#include "stat/weighted_statistic.hpp"
#include "stat/confidence_interval.hpp"

#include <atomic>
#include <QObject>

struct refresh_info
{
	size_t workers_1_working;
	size_t workers_2_working;
	size_t customer_queue_length;
	size_t cars_waiting_for_repair_queue_length;
	size_t repaired_cars_queue_length;
	double average_customer_queue_length;
	double average_cars_waiting_for_repair_queue_length;
	double average_repaired_cars_queue_length;
	double average_wait_in_queue_duration;
	double average_wait_for_repair_duration;
	double average_time_in_service_duration;
};

struct replication_info
{
	double average_customer_queue_length;
	double average_cars_waiting_for_repair_queue_length;
	double average_repaired_cars_queue_length;
	double average_wait_in_queue_duration;
	double average_wait_for_repair_duration;
	double average_time_in_service_duration;
	Interval wait_for_repair_90_CI;
	Interval time_in_service_90_CI;
	Interval wait_in_queue_90_CI;
};

class sim_core_car_service : public QObject, public sim_core_base
{
	Q_OBJECT

public:
	sim_core_car_service();
	virtual ~sim_core_car_service();

	void init(uint32_t group1_workers, uint32_t group2_workers, Seed seed = std::random_device{}());

	void single_run(uint32_t replications, double max_time, uint32_t w1, uint32_t w2);
	void single_run_seed(uint32_t replications, double max_time, uint32_t w1, uint32_t w2, Seed seed);
	void multi_run(uint32_t replications, double max_time, uint32_t w1_min, uint32_t w1_max, uint32_t w2_min, uint32_t w2_max);
	void multi_run_seed(uint32_t replications, double max_time, uint32_t w1_min, uint32_t w1_max, uint32_t w2_min, uint32_t w2_max, Seed seed);

	void before_replication(uint32_t replication) override;
	void after_replication(uint32_t replication) override;
	void before_simulation() override;
	void after_simulation() override;

	void on_stopped() override;

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

	Seed get_seed() const;

	size_t get_workers_1_working() const;
	size_t get_workers_2_working() const;
	size_t get_customer_queue_len() const;
	size_t get_cars_waiting_for_repair_queue_len() const;
	size_t get_repaired_cars_queue_len() const;
	Interval get_wait_for_repair_duration_90_CI() const;

signals:
	void replication_started(int replication);
	void replication_finished(int replication, replication_info info);
	void simulation_started(int workers1, int workers2);
	void simulation_finished(int workers1, int workers2, double wait_for_repair_total_time, double wait_in_queue_total_time, double queue_len_total, double time_in_service_total);
	void best_worker_count_found(int w1, int w2);
	void run_finished();
	void refresh_triggered(double time, refresh_info info);

private:
	void _init_time() override;
	void _delete_event(sim_event_base *event) const override;
	void _clear_queue(std::queue<customer*> &queue);
	void _clear_all_queues();
	double _generate_repair_duration();
	double _error(const std::pair<uint32_t, uint32_t> &setting, const std::pair<double, double> &result);

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

	statistic _stat_wait_for_repair; // take all
	statistic _stat_wait_for_repair_total;
	statistic _stat_wait_in_queue; // take all
	statistic _stat_wait_in_queue_total;
	weighted_statistic _stat_queue_len; // take only before max_time
	statistic _stat_queue_len_total;
	weighted_statistic _stat_cars_for_repair_queue_len; // only before max_time
	statistic _stat_cars_for_repair_queue_len_total;
	weighted_statistic _stat_repaired_cars_queue_len; // only before max_time
	statistic _stat_repaired_cars_queue_len_total;
	statistic _stat_time_in_service; // take all
	statistic _stat_time_in_service_total;

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
