#include <stdio.h>

#define SEH_IMPL
#include "seh.h"

int main(int argc, char* argv[])
{
    int count = 100;
    while (count-- > 0)
    {
		printf("Before SEH\n");

        seh_try (seh)
		{
			int* ptr = NULL;
			*ptr = 0; /* OS will throw exception here */
		}
		seh_catch (seh_get() == SEH_SEGFAULT)
		{
			fprintf(stderr, "%d. Segment fault exception has been thrown\n", count);
		}
		seh_finally (seh)
		{
			printf("Finally of try/catch\n");
		}
    }
    
    return 0;
}
