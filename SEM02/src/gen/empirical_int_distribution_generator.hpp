#ifndef EMPIRICAL_INT_DISTRIBUTION_GENERATOR_HPP
#define EMPIRICAL_INT_DISTRIBUTION_GENERATOR_HPP

#include "sim/sim_settings.hpp"
#include <map>

template <typename IntType = int>
class empirical_int_distribution_generator
{
	static_assert(std::is_integral<IntType>::value, "template argument not an integral type");

public:
	using result_type = IntType;

public:
	struct interval_definition
	{
		const result_type min, max;
		const double prob;
	};

public:
	empirical_int_distribution_generator(const std::initializer_list<interval_definition> &ilist, Seed seed = std::random_device{}())
	{
		this->seed(seed);
		double sum = std::accumulate(ilist.begin(), ilist.end(), 0.0, [](double d, const interval_definition &inter) { return d + inter.prob; });
		double d = 0.0;
		for (const auto &inter : ilist)
		{
			d += inter.prob;
			_dists[d / sum] = std::uniform_int_distribution<result_type>(inter.min, inter.max);
		}
	}

	~empirical_int_distribution_generator()
	{
	}

	result_type operator()()
	{
		auto denom = static_cast<double>(_gen_interval.max() - _gen_interval.min() + static_cast<Seed>(1));
		auto nom = static_cast<double>(_gen_interval() - _gen_interval.min());
		auto u = nom / denom;
		auto &dist = _dists.upper_bound(u)->second;
		return dist(_gen_value);
	}

	void seed(Seed seed)
	{
		Generator seed_gen(seed);
		_gen_interval.seed(seed_gen());
		_gen_value.seed(seed_gen());
	}

private:
	Generator _gen_interval, _gen_value;
	std::map<double, std::uniform_int_distribution<result_type> > _dists;
};

#endif // EMPIRICAL_INT_DISTRIBUTION_GENERATOR_HPP
