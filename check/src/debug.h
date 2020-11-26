#ifndef EVOLVE_CHESS_LOG_H
#define EVOLVE_CHESS_LOG_H

struct debug_channel
{
	const char *name;
	int         enabled;
	int         multiline_mode;
};

#define STRINGIFY(x)  #x

#define CHANNEL_NAME(name) \
	channel_##name

#define DEFINE_DEBUG_CHANNEL(name, enabled) \
struct debug_channel CHANNEL_NAME(name) = \
{ \
	STRINGIFY(name), \
	enabled, \
	0 \
} \

#undef DEBUG

#ifndef _GNUC
static inline void DBG(struct debug_channel* channel_name, const char* fmt, ...)
{

}
#else
#ifdef DEBUG
#define DBG(channel_name, fmt, args...) \
	debug_print (&CHANNEL_NAME(channel_name), __PRETTY_FUNCTION__, \
	             __LINE__, fmt,  ## args)
#else
#define DBG(channel, fmt, args...)
#endif // _DEBUG
#endif // _GNUC

void debug_print(struct debug_channel* channel, const char* func, int line,
	const char* fmt, ...)
#ifdef _GNUC
	__attribute__((format(printf, 4, 5)));
#else
	;
#endif

void debug_multi_line_start (struct debug_channel *channel);
void debug_multi_line_stop  (struct debug_channel *channel);

#endif // EVOLVE_CHESS_LOG_H
