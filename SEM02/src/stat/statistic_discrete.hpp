#ifndef STATISTIC_DISCRETE_HPP
#define STATISTIC_DISCRETE_HPP

#include <cinttypes>

class statistic_discrete
{
public:
	statistic_discrete();
	~statistic_discrete();

	void add(double value);
	void clear();

	double mean() const;

private:
	double _sum;
	uint64_t _n;
};

#endif // STATISTIC_DISCRETE_HPP
