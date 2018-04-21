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

    __try
    {
	int* ptr = NULL;
	*ptr = 0; /* Throw exception here */
    }
    __except (seh_get_excode() != SEH_EXCODE_NONE)
    {
	fprintf(stderr, "Segment fault exception has been thrown\n");
    }
    __finally
    {
	fprintf(stderr, "An unknown exception has been thrown\n"); 	
    }

    /* @note: when try in seconds, software may be pause, fixing... */
    __try
    {
	int* ptr = NULL;
	*ptr = 0; /* Throw exception here */
    }
    __except (seh_get_excode() != SEH_EXCODE_NONE)
    {
	fprintf(stderr, "Segment fault exception has been thrown\n");
    }
    __finally
    {
	fprintf(stderr, "An unknown exception has been thrown\n"); 	
    }
    
    seh_quit();
    return 0;
}
