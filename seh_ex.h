#ifndef __SEH_EX_H__
#define __SEH_EX_H__

#include <setjmp.h>

#ifndef SEH_EX_API
#define SEH_EX_API
#endif

#ifndef SEH_LEAVE_VALUE
#define SEH_LEAVE_VALUE -9999 // SHARE with seh.h
#endif

typedef jmp_buf seh_t;

#define seh_try(ctx)     seh_ex__push(&ctx); if (setjmp(ctx) == 0)
#define seh_catch(exp)   else if ((seh_get() != SEH_LEAVE_VALUE) && (exp))
#define seh_finally(ctx) seh_ex__pop(&ctx);
//#define seh_throw(i)     cur_value = i; longjmp(ctx, 1)

SEH_EX_API int  seh_get(void);
SEH_EX_API void seh_leave(void);
SEH_EX_API void seh_throw(int value);

// Internal functions

SEH_EX_API void seh_ex__pop(seh_t* ctx);
SEH_EX_API void seh_ex__push(seh_t* ctx);

#endif /* __SEH_EX_H__ */

#ifdef SEH_EX_IMPL

#ifndef SEH_EX_STACK_SIZE
#define SEH_EX_STACK_SIZE 64
#endif

static int    seh_value;
static int    seh_stack_pointer;
static seh_t* seh_stack[SEH_EX_STACK_SIZE];

int seh_get(void)
{
    return seh_value;
}

void seh_leave(void)
{
    seh_throw(SEH_LEAVE_VALUE);
}

void seh_throw(int value)
{
    seh_value = value;
    seh_t* ctx = seh_stack[--seh_stack_pointer];
    longjmp(*ctx, 1);
}

void seh_ex__pop(seh_t* ctx)
{
    seh_stack_pointer -= (ctx == seh_stack[seh_stack_pointer]);
}

void seh_ex__push(seh_t* ctx)
{
    seh_stack[seh_stack_pointer++] = ctx;
}

#endif /* SEH_IMPL */