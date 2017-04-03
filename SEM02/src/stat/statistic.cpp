#include "statistic.hpp"

#include <cmath>

statistic::statistic()
	: _sum(0.0), _sum2(0.0), _n(0)
{
}

statistic::~statistic()
{
}

void statistic::add(double value)
{
	_sum += value;
	_sum2 += value * value;
	++_n;
}

void statistic::clear()
{
	_sum = 0.0;
	_sum2 = 0.0;
	_n = 0;
}

uint64_t statistic::n() const
{
	return _n;
}

double statistic::mean() const
{
	return _n == 0 ? 0.0 : _sum / _n;
}

double statistic::variance() const
{
	return _n == 0 ? 0.0 : (_sum2 / _n) - std::pow(mean(), 2.0);
}
