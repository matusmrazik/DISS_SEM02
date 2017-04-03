#include "confidence_interval.hpp"

#include <array>
#include <map>
#include <limits>
#include <cmath>

#define INF     std::numeric_limits<double>::infinity()
#define U32MAX  std::numeric_limits<uint32_t>::max()

std::array<double, 31> quantiles_30 {
	INF,   6.314, 2.920, 2.353, 2.132, 2.015, 1.943, 1.895, 1.860, 1.833,
	1.812, 1.796, 1.782, 1.771, 1.761, 1.753, 1.746, 1.740, 1.734, 1.729,
	1.725, 1.721, 1.717, 1.714, 1.711, 1.708, 1.706, 1.703, 1.701, 1.699
};

std::map<std::uint32_t, double> quantiles {
	{ 30, 1.697 },
	{ 40, 1.684 },
	{ 50, 1.676 },
	{ 60, 1.671 },
	{ 70, 1.667 },
	{ 80, 1.664 },
	{ 90, 1.662 },
	{ 100, 1.660 },
	{ 1000, 1.646 },
	{ UINT32_MAX, 1.645 }
};

Interval confidence_interval_90_percent(uint32_t n, double mean, double variance)
{
	double critval;
	if (n < 30)
	{
		critval = quantiles_30[n];
	}
	else if (n > 1000)
	{
		critval = 1.645;
	}
	else
	{
		auto it = quantiles.lower_bound(n);
		if (it->first == n)
		{
			critval = it->second;
		}
		else
		{
			auto itnext = it;
			std::advance(it, -1);
			critval = (double(n - it->first) / (itnext->first - it->first)) * (itnext->second - it->second);
		}
	}
	double stddev = std::sqrt(variance);
	double diff = n == 1u ? 0.0 : (critval * stddev) / std::sqrt(n - 1);
	return std::make_pair(mean - diff, mean + diff);
}

Interval confidence_interval_90_percent(const statistic &stat)
{
	return confidence_interval_90_percent(stat.n(), stat.mean(), stat.variance());
}
