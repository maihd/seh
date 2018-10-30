//
// AN IMPLEMENTATION FOR SIGJMP/LONGJMP
// Status: failed
//

#include <stdio.h>

static struct
{
    int  lr;
    int  fp;
    int* sp;
} ctx;

static void setjmp_leaf() {}

int setjmp(int arg)
{
    setjmp_leaf();

    ctx.sp = &arg;
    
    ctx.lr = (&arg)[-1];

    ctx.fp = (&arg)[-2];

    return 0;
}

static void longjmp_leaf(int arg)
{
    (&arg)[-1] = (int)(intptr_t)ctx.sp;
}

void longjmp(int arg, int value)
{
    longjmp_leaf(arg);

    (&arg)[-1] = ctx.lr;
    (&arg)[-2] = ctx.fp;

    return;
}

int main(int argc, char* argv[])
{
    int buf = 0;
    
    if (setjmp(buf) == 0)
    {
        longjmp(buf, 10);
    }
    else
    {
        printf("longjmp to here\n");
    }

    return 0;
}