#include "weighted_statistic.hpp"

weighted_statistic::weighted_statistic()
	: _sum(0.0), _cur_value(0.0), _cur_weight(0.0)
{
}

weighted_statistic::~weighted_statistic()
{
}

void weighted_statistic::add(double value, double weight)
{
	_sum += _cur_value * (weight - _cur_weight);
	_cur_value = value;
	_cur_weight = weight;
}

void weighted_statistic::clear()
{
	_sum = 0.0;
	_cur_value = 0.0;
	_cur_weight = 0.0;
}

double weighted_statistic::mean() const
{
	return _cur_weight <= 0.0 ? 0.0 : _sum / _cur_weight;
}

double weighted_statistic::mean(double weight) const
{
	return weight <= 0.0 ? 0.0 : (_sum + _cur_value * (weight - _cur_weight)) / weight;
}
