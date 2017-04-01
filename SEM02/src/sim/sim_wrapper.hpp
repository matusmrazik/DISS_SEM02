#ifndef SIM_WRAPPER_HPP
#define SIM_WRAPPER_HPP

#include "sim_core_car_service.hpp"

class sim_wrapper : public QObject
{
	Q_OBJECT

public:
	sim_wrapper(uint32_t workers1, uint32_t workers2, uint32_t replications, double max_time);
	~sim_wrapper();

	void find_best_solution(uint32_t w1_min, uint32_t w1_max, uint32_t w2_min, uint32_t w2_max);
	void run();

private slots:
	void simulation_finished(double wait_for_repair_time, double wait_in_queue_time);

private:
	double _error(const std::pair<uint32_t, uint32_t> &setting, const std::pair<double, double> &result);

private:
	uint32_t _workers1, _workers2;
	uint32_t _replications;
	double _max_time;
	std::pair<double, double> _cur_result, _best_result;
	std::pair<uint32_t, uint32_t> _best_setting;
	sim_core_car_service _sim;
};

#endif // SIM_WRAPPER_HPP
