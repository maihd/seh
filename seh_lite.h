#ifndef __SEH_LITE_H__
#define __SEH_LITE_H__

#include <setjmp.h>

#ifndef SEH_LITE_API
#define SEH_LITE_API
#endif

#ifndef SEH_LEAVE
#define SEH_LEAVE -9999 // SHARE with seh.h
#endif

typedef jmp_buf seh_lite_t;

#define seh_lite_try(ctx)     seh_lite__push(&ctx); if (setjmp(ctx) == 0)
#define seh_lite_leave()      seh_lite_throw(SEH_LEAVE)
#define seh_lite_catch(exp)   else if ((seh_lite_get() != SEH_LEAVE) && (exp))
#define seh_lite_finally(ctx) seh_lite__pop(&ctx);
//#define seh_throw(i)     cur_value = i; longjmp(ctx, 1)

SEH_LITE_API int  seh_lite_get(void);
SEH_LITE_API void seh_lite_throw(int value);

// Internal functions

SEH_LITE_API void seh_lite__pop(seh_lite_t* ctx);
SEH_LITE_API void seh_lite__push(seh_lite_t* ctx);

#endif /* __SEH_LITE_H__ */

#ifdef SEH_LITE_IMPL

#ifndef SEH_LITE_STACK_SIZE
#define SEH_LITE_STACK_SIZE 64
#endif

static int          seh_lite_value;
static int          seh_lite_stack_pointer;
static seh_lite_t*  seh_lite_stack[SEH_LITE_STACK_SIZE];

int seh_lite_get(void)
{
    return seh_lite_value;
}

void seh_lite_throw(int value)
{
    seh_lite_value = value;
    seh_lite_t* ctx = seh_lite_stack[seh_lite_stack_pointer];
    longjmp(*ctx, 1);
}

void seh_lite__pop(seh_lite_t* ctx)
{
    seh_lite_stack_pointer -= (ctx == seh_lite_stack[seh_lite_stack_pointer]);
}

void seh_lite__push(seh_lite_t* ctx)
{
    seh_lite_stack[seh_lite_stack_pointer++] = ctx;
}

#endif /* SEH_LITE_MPL */