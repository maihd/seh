#include "seh.h"
#include <stdio.h>

#define SEH_IMPL
#include "seh.h"

int main(int argc, char* argv[])
{
    int count = 100;
    while (count-- > 0)
    {
		seh_t seh;
        seh_try (seh)
		{
			int* ptr = NULL;
			*ptr = 0; /* Throw exception here */
		}
		seh_catch (seh_get() == SEH_SEGFAULT)
		{
			fprintf(stderr, "Segment fault exception has been thrown\n");
		}
		seh_finally (seh)
		{
			printf("Finally of try/catch\n");
		}
    }
    
    return 0;
}
