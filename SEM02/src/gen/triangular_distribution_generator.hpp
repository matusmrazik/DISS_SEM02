#ifndef TRIANGULAR_DISTRIBUTION_GENERATOR_HPP
#define TRIANGULAR_DISTRIBUTION_GENERATOR_HPP

#include "sim/sim_settings.hpp"

template <typename RealType = double>
class triangular_distribution_generator
{
	static_assert(std::is_floating_point<RealType>::value, "template argument not a floating point type");

public:
	using result_type = RealType;

public:
	triangular_distribution_generator(result_type min = -1, result_type mod = 0, result_type max = 1, Seed seed = std::random_device{}())
		: _min(min), _mod(mod), _max(max), _gen(seed)
	{
	}

	~triangular_distribution_generator()
	{
	}

	result_type operator()()
	{
		auto denom = static_cast<result_type>(_gen.max() - _gen.min() + static_cast<Seed>(1));
		auto nom = static_cast<result_type>(_gen() - _gen.min());
		result_type u = nom / denom;
		result_type f = (_mod - _min) / (_max - _min);
		if (u < f)
			return _min + std::sqrt(u * (_mod - _min) * (_max - _min));
		return _max - std::sqrt((static_cast<result_type>(1) - u) * (_max - _min) * (_max - _mod));
	}

	void seed(Seed seed)
	{
		_gen.seed(seed);
	}

private:
	result_type _min, _mod, _max;
	Generator _gen;
};

#endif // TRIANGULAR_DISTRIBUTION_GENERATOR_HPP
