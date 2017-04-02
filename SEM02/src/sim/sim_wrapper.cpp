#include "sim_wrapper.hpp"

#include <limits>

sim_wrapper::sim_wrapper(uint32_t workers1, uint32_t workers2, uint32_t replications, double max_time)
	: _workers1(workers1), _workers2(workers2), _replications(replications), _max_time(max_time),
	  _cur_result(0.0, 0.0), _best_result(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
	  _best_setting(0, 0)
{
	QObject::connect(&_sim, SIGNAL(simulation_finished(double,double)), this, SLOT(simulation_finished(double,double)));
}

sim_wrapper::~sim_wrapper()
{
}

void sim_wrapper::find_best_solution(uint32_t w1_min, uint32_t w1_max, uint32_t w2_min, uint32_t w2_max)
{
	for (uint32_t w1 = w1_min; w1 <= w1_max; ++w1)
	{
		for (uint32_t w2 = w2_min; w2 <= w2_max; ++w2)
		{
			printf("Running with {%u, %u}...\n", w1, w2);
			_sim.init(w1, w2);
			_sim.simulate(_replications, _max_time);
			if (_error({w1, w2}, _cur_result) < _error(_best_setting, _best_result))
			{
				_best_result = _cur_result;
				_best_setting = {w1, w2};
			}
		}
	}
	printf("Najlepsi pocet pracovnikov: %u v 1. skupine, %u v 2. skupine\n", _best_setting.first, _best_setting.second);
	printf("Priemerny cas cakania na opravu: %s\n", duration_as_string(_best_result.first).toUtf8().data());
	printf("Priemerny cas cakania v rade: %s\n", duration_as_string(_best_result.second).toUtf8().data());
}

void sim_wrapper::run()
{
	_sim.init(_workers1, _workers2);
	_sim.simulate(_replications, _max_time);
	printf("Priemerny cas cakania na opravu: %s\n", duration_as_string(_cur_result.first).toUtf8().data());
	printf("Priemerny cas cakania v rade: %s\n", duration_as_string(_cur_result.second).toUtf8().data());
}

void sim_wrapper::simulation_finished(double wait_for_repair_time, double wait_in_queue_time)
{
	_cur_result = {wait_for_repair_time, wait_in_queue_time};
}

double sim_wrapper::_error(const std::pair<uint32_t, uint32_t> &setting, const std::pair<double, double> &result) // TODO upravit na zaklade poctu zamestnancov
{
	if (result.first > hours(5.0)) return std::numeric_limits<double>::max();
	if (result.second > minutes(3.0)) return std::numeric_limits<double>::max();
//	return std::pow(result.first - hours(5.0), 2.0) + std::pow(result.second - minutes(3.0), 2.0);
	return setting.first + setting.second;
}
