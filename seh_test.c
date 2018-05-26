#include "seh.h"
#include <stdio.h>

#define SEH_IMPL
#include "seh.h"

int main(int argc, char* argv[])
{
    if (seh_init() != SEH_INIT_SUCCESS)
    {
	return 1;
    }

    int count = 100;
    while (count-- > 0)
    {
        __try
	{
	    int* ptr = NULL;
	    *ptr = 0; /* Throw exception here */
	}
	__except (seh_get_excode() == SEH_EXCODE_SEGFAULT)
	{
	    fprintf(stderr, "Segment fault exception has been thrown\n");
	}
	__except (seh_get_excode() > SEH_EXCODE_LEAVE) /* Ignore __leave */
	{
	    fprintf(stderr, "An unknown exception has been thrown\n"); 	
	}
	__finally
	{
	    printf("Finally of try/except\n");
	}
	__finally
    }
    
    seh_quit();
    return 0;
}
