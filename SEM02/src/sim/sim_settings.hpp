#ifndef SIM_SETTINGS_HPP
#define SIM_SETTINGS_HPP

#include <random>
#include <cstdio>
#include <cstring>

using Generator = std::default_random_engine;
using Seed = Generator::result_type;

constexpr double WORKDAY_HOURS          = 8.0;
constexpr double HEAT_UP_HOURS          = 0.5;

template <typename T>
constexpr inline T minutes(const T &mins)
{
	return mins * static_cast<T>(60);
}

template <typename T>
constexpr inline T hours(const T &hrs)
{
	return hrs * static_cast<T>(3600);
}

template <typename T>
constexpr inline T days(const T &d)
{
	return hours(d * static_cast<T>(WORKDAY_HOURS));
}

template <typename T>
constexpr inline double to_minutes(const T &time)
{
	return static_cast<double>(time) / 60.0;
}

template <typename T>
constexpr inline double to_hours(const T &time)
{
	return static_cast<double>(time) / 3600.0;
}

template <typename T>
constexpr inline double to_days(const T &time)
{
	return to_hours(time) / WORKDAY_HOURS;
}

inline std::string to_pretty_str(const double time)
{
	auto s = static_cast<int>(time);
	double seconds = time - s + (s % 60);
	int minutes = (s / 60) % 60;
	int hours = (s / 3600) % 24;
	int days = static_cast<int>(to_days(s));
	char str[30];
	std::memset(str, 0, 30);
	sprintf(str, "%d:%02d:%02d:%09.06f", days, hours, minutes, seconds);
	return std::string(str);
}

constexpr double TIME_BETWEEN_CUSTOMERS = minutes(5.0);

#endif // SIM_SETTINGS_HPP
