#ifndef SIM_SETTINGS_HPP
#define SIM_SETTINGS_HPP

#include <random>

using Generator = std::default_random_engine;
using Seed = Generator::result_type;

constexpr double WORKDAY_HOURS          = 8.0;
constexpr double HEAT_UP_HOURS          = 0.5;

template <typename T>
constexpr inline T minutes(T mins)
{
	return mins * static_cast<T>(60);
}

template <typename T>
constexpr inline T hours(T hrs)
{
	return hrs * static_cast<T>(3600);
}

template <typename T>
constexpr inline T days(T d)
{
	return hours(d * static_cast<T>(WORKDAY_HOURS));
}

constexpr double TIME_BETWEEN_CUSTOMERS = minutes(5.0);

#endif // SIM_SETTINGS_HPP
