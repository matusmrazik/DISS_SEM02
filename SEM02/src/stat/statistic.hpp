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

	double mean() const;

private:
	double _sum;
	uint64_t _n;
};

#endif // STATISTIC_HPP
