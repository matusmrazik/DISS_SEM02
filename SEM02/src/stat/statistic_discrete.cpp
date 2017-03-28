#include "statistic_discrete.hpp"

statistic_discrete::statistic_discrete()
	: _sum(0.0), _n(0)
{
}

statistic_discrete::~statistic_discrete()
{
}

void statistic_discrete::add(double value)
{
	_sum += value;
	++_n;
}

void statistic_discrete::clear()
{
	_sum = 0.0;
	_n = 0;
}

double statistic_discrete::mean() const
{
	return _sum / _n;
}
