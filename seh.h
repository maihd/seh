#ifndef __SEH_H__
#define __SEH_H__

#ifndef __seh__
#define __seh__
#endif

#if !defined(_MSC_VER)
# include <setjmp.h>

# if defined(_WIN32)
extern jmp_buf seh__jmpenv;
# define seh__setjmp() setjmp(seh__jmpenv)
# else
extern sigjmp_buf seh__jmpenv;
# define seh__setjmp() sigsetjmp(seh__jmpenv, 0)
# endif

/* Should call __leave instead */
__seh__ void seh__leave(void);

/* The syntax */

# define __try          if (seh__setjmp() == 0)
# define __except(cond) else if (cond)
# define __finally      
# define __leave        seh__leave()
#endif /* !defined(_MSC_VER) */

/**
 * Exception code
 */
enum
{
    SEH_EXCODE_NONE,
    SEH_EXCODE_LEAVE,
    SEH_EXCODE_ABORT,
    SEH_EXCODE_FLOAT,
    SEH_EXCODE_SYSCALL,
    SEH_EXCODE_ILLCODE,
    SEH_EXCODE_MISALIGN,
    SEH_EXCODE_SEGFAULT,
    SEH_EXCODE_OUTBOUNDS,
    SEH_EXCODE_STACKOVERFLOW,
};

/**
 * Initialize code
 */
enum
{
    SEH_INIT_SUCCESS,
    SEH_INIT_SIGNAL_FAILED,
};

__seh__ int  seh_init(void);
__seh__ void set_quit(void);
__seh__ int  seh_get_excode(void);

#endif /* __SEH_H__ */


#if defined(SEH_IMPL)

static int seh__excode = SEH_EXCODE_NONE;

#if defined(_WIN32)
#include <Windows.h>
static void seh__filter(int excode)
{
    switch (excode)
    {
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_INVALID_OPERATION:
	seh__excode = SEH_EXCODE_FLOAT;
	break;

    case EXCEPTION_ILLEGAL_INSTRUCTION:
	seh__excode = SEH_EXCODE_ILLCODE;
	break;

    case EXCEPTION_STACK_OVERFLOW:
	seh__excode = SEH_EXCODE_STACKOVERFLOW;
	break;
	
    case EXCEPTION_ACCESS_VIOLATION:
	seh__excode = SEH_EXCODE_SEGFAULT;
	break;
	
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	seh__excode = SEH_EXCODE_OUTBOUNDS;
	break;

    case EXCEPTION_DATATYPE_MISALIGNMENT:
	seh__excode = SEH_EXCODE_MISALIGN;
	break;
	
    default:
	seh__excode = SEH_EXCODE_NONE;
	break;
    }
}
#endif

#if defined(_MSV_VER)
int seh_init(void)
{
    return SEH_INIT_SUCCESS;
}

void seh_quit(void)
{
    seh__filter(GetExceptionCode());
    return seh__excode;
}

int seh_get_excode(void)
{
}
#else

# define seh__countof(x) (sizeof(x) / sizeof((x)[0]))

# if defined(_WIN32)
jmp_buf seh__jmpenv;
# define seh__longjmp() longjmp(seh__jmpenv, seh__excode)
# else
sigjmp_buf seh__jmpenv;
const int seh__signals[] = {
    SIGABRT, SIGFPE, SIGSEGV, SIGILL, SIGSYS, SIGBUS,
};
# define seh__longjmp() siglongjmp(seh__jmpenv, seh__excode)
# endif

#if defined(_WIN32)
static LONG WINAPI seh__sighandler(EXCEPTION_POINTERS* info)
{
    seh__filter(info->ExceptionRecord->ExceptionCode);
    seh__longjmp();
    return seh__excode;
}

int seh_init(void)
{
    SetUnhandledExceptionFilter(seh__sighandler);
    return SEH_INIT_SUCCESS;
}

void seh_quit(void)
{
    SetUnhandledExceptionFilter(NULL);
}
#else
static void seh__sighandler(int sig, siginfo_t* info, void* context)
{
    (void)info;
    (void)context;
    switch (sig)
    {
    case SIGBUS:
	seh__excode = SEH_EXCODE_MISALIGN;
	break;

    case SIGSYS:
	seh__excode = SEH_EXCODE_SYSCALL;
	break;

    case SIGFPE:
	seh__excode = SEH_EXCODE_FLOAT;
	break;
	
    case SIGILL:
	seh__excode = SEH_EXCODE_ILLCODE;
	break;

    case SIGABRT:
	seh__excode = SEH_EXCODE_ABORT;
	break;

    case SIGSEGV:
	seh__excode = SEH_EXCODE_SEGFAULT;
	break;
	
    default:
	seh__excode = SEH_EXCODE_NONE;
	break;
    }

    seh__longjmp();
}

int seh_init(void)
{
    int idx;
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler   = NULL;
    sa.sa_sigaction = seh__sighandler;
    sa.sa_flags     = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    for (idx = 0; idx < seh__countof(seh__signals); idx++)
    {
	if (sigaction(seh__signals[idx], &sa, NULL))
	{
	    return SEH_INIT_SIGNAL_FAILED;
	}
    }
    return SEH_INIT_SUCCESS;
}

void seh_quit(void)
{
    int idx;
    for (idx = 0; idx < seh__countof(seh__signals); idx++)
    {
	if (signal(seh__signals[idx], SIG_DFL))
	{
	    break;
	}
    }
}
#endif

int seh_get_excode(void)
{
    return seh__excode;
}

void seh__leave(void)
{
    seh__excode = SEH_EXCODE_LEAVE;
    seh__longjmp();
}
#endif /* defined(_MSC_VER) */

#endif /* defined(SEH_IMPL) */
