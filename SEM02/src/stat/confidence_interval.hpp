#ifndef CONFIDENCE_INTERVAL_HPP
#define CONFIDENCE_INTERVAL_HPP

#include "statistic.hpp"
#include <utility>
#include <cstdint>

using Interval = std::pair<double, double>;

Interval confidence_interval_90_percent(uint32_t n, double mean, double variance);
Interval confidence_interval_90_percent(const statistic &stat);

#endif // CONFIDENCE_INTERVAL_HPP
