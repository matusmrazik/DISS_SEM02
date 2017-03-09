#include "log.hpp"

#ifdef DEBUG

Log::Log()
	: _stream(fopen(LOG_FILE_NAME ".log", "a"))
{
}

Log::~Log()
{
	fclose(_stream);
}

Log &Log::instance()
{
	static Log inst;
	return inst;
}

#endif
