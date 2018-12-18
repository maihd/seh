#include <stdio.h>

#define SEH_LITE_IMPL
#include "./seh_lite.h"

int main(int argc, char* argv[])
{
    seh_lite_t ctx; 
    seh_lite_try (ctx)
    {
        printf("prepare to throw an error\n");
        seh_lite_throw(1);
        printf("should not should this\n");
    }
    seh_lite_catch (seh_lite_get() == 1)
    {
        printf("catch an error that threw with value=1.\n");
    }
    seh_lite_finally (ctx)
    {
        printf("finally we done.\n");
    }

    return 0;
}