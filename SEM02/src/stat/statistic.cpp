#include "statistic.hpp"

statistic::statistic()
	: _sum(0.0), _n(0)
{
}

statistic::~statistic()
{
}

void statistic::add(double value)
{
	_sum += value;
	++_n;
}

void statistic::clear()
{
	_sum = 0.0;
	_n = 0;
}

double statistic::mean() const
{
	return _sum / _n;
}
