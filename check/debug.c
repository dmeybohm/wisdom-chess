#include <stdarg.h>
#include <stdio.h>

#include "debug.h"

static FILE *outputf;

void debug_print (struct debug_channel *channel, char *func,
                  int line, char *fmt, ...)
{
	va_list args;

	if (!channel->enabled)
		return;

	if (!outputf)
		outputf = stdout;

	va_start (args, fmt);

	if (!channel->multiline_mode)
	{
		fprintf (outputf, "%-10s: %-9s: %-4i: ", 
		         channel->name, func, line);
	}
	
	vfprintf (outputf, fmt, args);

	va_end (args);
}

void debug_multi_line_start (struct debug_channel *channel)
{
	channel->multiline_mode = 1;
}

void debug_multi_line_stop (struct debug_channel *channel)
{
	channel->multiline_mode = 0;
}

/* vi: set ts=4 sw=4: */
