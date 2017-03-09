#ifndef LOG_HPP
#define LOG_HPP

#ifdef DEBUG

#include <cstdio>
#include <ctime>
#include <utility>

class Log
{
public:
	~Log();

	static Log &instance();

	template <typename... Args>
	void log(const char *format, int line, Args&&... args);
private:
	Log();

	FILE *_stream;
};

template <typename... Args>
void Log::log(const char *format, int line, Args&&... args)
{
	std::time_t time = std::time(nullptr);
	struct tm *tm = std::localtime(&time);

	char time_str[20];
	snprintf(time_str, 20, "%02d.%02d.%04d %02d:%02d:%02d", tm->tm_mday, tm->tm_mon, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);

	fprintf(_stream, format, time_str, line, std::forward<Args...>(args)...);
}

#define __LOG_FORMAT_HELPER(fmt, log_type)  log_type " [%s] [" __FILE__ ":line %d] " fmt "\n"

#define LOG_INFO(fmt, args...) Log::instance().log(__LOG_FORMAT_HELPER(fmt, "I"), __LINE__ , ##args)
#define LOG_WARN(fmt, args...) Log::instance().log(__LOG_FORMAT_HELPER(fmt, "W"), __LINE__ , ##args)
#define LOG_ERROR(fmt, args...) Log::instance().log(__LOG_FORMAT_HELPER(fmt, "E"), __LINE__ , ##args)

#else

#define LOG_INFO(fmt, args...)
#define LOG_WARN(fmt, args...)
#define LOG_ERROR(fmt, args...)

#endif

#endif // LOG_HPP
