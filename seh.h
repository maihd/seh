#ifndef __SEH_H__
#define __SEH_H__

#include <setjmp.h>

#ifndef SEH_API
#define SEH_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Exception code
 */
#define SEH_NONE            0
#define SEH_LEAVE           -0x9999
#define SEH_ABORT           -0x22222
#define SEH_FLOAT           -0x11111
#define SEH_SYSCALL         -0x33333
#define SEH_ILLCODE         -0x12313
#define SEH_MISALIGN        -0x12341
#define SEH_SEGFAULT        -0x54647
#define SEH_OUTBOUNDS       -0xada32
#define SEH_STACKOVERFLOW   -0xdeadf

typedef struct seh
{
    struct seh* prev;
    int         value;
    void*       saved;
    jmp_buf     jmpbuf;
} seh_t;

#define seh_try(ctx)        seh_t* ctx = seh_begin(); if (setjmp((ctx)->jmpbuf) == 0)
#define seh_catch(exp)      else if ((seh_get() != SEH_LEAVE) && (exp))
#define seh_finally(ctx)    seh_end(ctx); if (1)

SEH_API int                 seh_get(void);
SEH_API void                seh_leave(void);
SEH_API void                seh_throw(int value);

SEH_API seh_t*              seh_begin(void);
SEH_API void                seh_end(seh_t* ctx);

#endif /* __SEH_H__ */

#ifdef SEH_IMPL

static seh_t* seh_curr;

#if defined(_WIN32)
#include <Windows.h>
static LONG WINAPI seh__sighandler(EXCEPTION_POINTERS* info)
{
    switch (info->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_INVALID_OPERATION:
        seh_throw(SEH_FLOAT);
        //seh_value = SEH_FLOAT;
        break;

    case EXCEPTION_ILLEGAL_INSTRUCTION:
        seh_throw(SEH_ILLCODE);
        //seh_value = SEH_ILLCODE;
        break;

    case EXCEPTION_STACK_OVERFLOW:
        seh_throw(SEH_STACKOVERFLOW);
        //seh_value = SEH_STACKOVERFLOW;
        break;
	
    case EXCEPTION_ACCESS_VIOLATION:
        seh_throw(SEH_SEGFAULT);
        //seh_value = SEH_SEGFAULT;
        break;
	
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        seh_throw(SEH_OUTBOUNDS);
        //seh_value = SEH_OUTBOUNDS;
        break;

    case EXCEPTION_DATATYPE_MISALIGNMENT:
        seh_throw(SEH_MISALIGN);
        //seh_value = SEH_MISALIGN;
        break;
	
    default:
        seh_throw(SEH_NONE);
        //seh_value = SEH_NONE;
        break;
    }

    return seh_curr && seh_curr->value != SEH_LEAVE 
        ? EXCEPTION_CONTINUE_EXECUTION              // The system stop handling error filter, return to execution checkpoint.
        : EXCEPTION_CONTINUE_SEARCH;                // The system continues to search for a handler.
}
#else
#include <signal.h>
#include <stdlib.h>

static void seh__sighandler(int sig, siginfo_t* info, void* context)
{
    (void)info;
    (void)context;
    switch (sig)
    {
    case SIGBUS:
        seh_throw(SEH_MISALIGN);
        break;

    case SIGSYS:
        seh_throw(SEH_SYSCALL);
        break;

    case SIGFPE:
        seh_throw(SEH_FLOAT);
        break;
	
    case SIGILL:
        seh_throw(SEH_ILLCODE);
        break;

    case SIGABRT:
        seh_throw(SEH_ABORT);
        break;

    case SIGSEGV:
        seh_throw(SEH_SEGFAULT);
        break;
	
    default:
        seh_throw(SEH_NONE);
        break;
    }
}
#endif


int seh_get(void)
{
    return seh_curr ? seh_curr->value : SEH_LEAVE;
}


void seh_leave(void)
{
    seh_throw(SEH_LEAVE);
}


void seh_throw(int value)
{
    if (seh_curr)
    {
        seh_curr->value = value;
        longjmp(seh_curr->jmpbuf, 1);
    }
}


#if !defined(_WIN32)
static const int seh_signals[] = {
    SIGABRT, SIGFPE, SIGSEGV, SIGILL, SIGSYS, SIGBUS,
};

static const int seh_signals_count = sizeof(seh_signals) / sizeof(seh_signals[0]);
#endif


seh_t* seh_begin(void)
{
#if defined(_WIN32)
    seh_t* ctx = (seh_t*)malloc(sizeof(seh_t));
    ctx->prev  = NULL;
    ctx->value = SEH_NONE;
    ctx->saved = (void*)SetUnhandledExceptionFilter(seh__sighandler);
#else
    seh_t* ctx = (seh_t*)malloc(sizeof(seh_t) + sizeof(struct sigaction) * seh_signals_count);
    ctx->prev  = NULL;
    ctx->value = SEH_NONE;
    ctx->saved = (char*)ctx + sizeof(seh_t);

    int idx;
    struct sigaction sa, old;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler   = NULL;
    sa.sa_sigaction = seh__sighandler;
    sa.sa_flags     = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    for (idx = 0; idx < seh_signals_count; idx++)
    {
        if (sigaction(seh_signals[idx], &sa, &((struct sigaction*)ctx->saved)[idx]) != 0)
        {
            free(ctx);
            return NULL;
        }
    }
#endif

    ctx->prev = seh_curr;
    seh_curr = ctx;
    return ctx;
}


void seh_end(seh_t* ctx)
{
    if (ctx == seh_curr)
    {
    #if defined(_WIN32)
        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ctx->saved);
    #else
        int idx;
        for (idx = 0; idx < seh_signals_count; idx++)
        {
            if (sigaction(seh_signals[idx], &((struct sigaction*)ctx->saved)[idx], NULL) != 0)
            {
                break;
            }
        }
    #endif

        seh_curr = seh_curr->prev;
        free(ctx);
    }
}

#endif /* SEH_IMPL */

#ifdef __cplusplus
}
#endif

//! EOF
