#include "defs.h"

void memset(BYTE* s, BYTE c, int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		*s = c;
		s++;
	}
}