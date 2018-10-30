#include <stdio.h>

#define SEH_EX_IMPL
#include "./seh_ex.h"

int main(int argc, char* argv[])
{
    seh_ctx_t ctx; 
    seh_try (ctx)
    {
        printf("prepare to throw an error\n");
        seh_throw(1);
        printf("should not should this\n");
    }
    seh_catch (seh_get() == 1)
    {
        printf("catch an error that threw with value=1.\n");
    }
    seh_finally (ctx)
    {
        printf("finally we done.\n");
    }

    return 0;
}