#ifndef STATISTIC_HPP
#define STATISTIC_HPP

#include <cinttypes>

class statistic
{
public:
	statistic();
	~statistic();

	void add(double value);
	void clear();

	uint64_t n() const;
	double mean() const;
	double variance() const;

private:
	double _sum;
	double _sum2;
	uint64_t _n;
};

#endif // STATISTIC_HPP
