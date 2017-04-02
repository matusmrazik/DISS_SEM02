#ifndef SIM_SETTINGS_HPP
#define SIM_SETTINGS_HPP

#include <random>
#include <QString>

using Generator = std::default_random_engine;
using Seed = Generator::result_type;

constexpr uint32_t WORKDAY_HOURS        = 8;
constexpr uint32_t WORKDAY_START_HOUR   = 7;
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
	return to_hours(time) / static_cast<double>(WORKDAY_HOURS);
}

inline QString duration_as_string(const double time)
{
	auto s = static_cast<int>(time);
	double seconds = time - s + (s % 60);
	int minutes = (s / 60) % 60;
	int hours = s / 3600;
	return QString::asprintf("%02d h, %02d min, %05.02f s", hours, minutes, seconds);
}

inline QString sim_time_as_string(const double time)
{
	if (time < 0.0)
		return "Zahrievanie...";

	auto s = static_cast<int>(time);
	double seconds = time - s + (s % 60);
	int minutes = (s / 60) % 60;
	int hours = WORKDAY_START_HOUR + ((s / 3600) % WORKDAY_HOURS);
	int days = static_cast<int>(to_days(time)) + 1;
	return QString::asprintf("deň %d, %02d:%02d:%05.02f", days, hours, minutes, seconds);
}

constexpr double TIME_BETWEEN_CUSTOMERS = minutes(5.0);

#endif // SIM_SETTINGS_HPP
