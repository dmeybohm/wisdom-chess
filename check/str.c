#include <string.h>
#include <stdio.h>

void chomp (char *str)
{
	if (str)
	{
		size_t len = strlen (str);

		if (str[len-1] == '\n')
			str[len] = 0;
	}
}
