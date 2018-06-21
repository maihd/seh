#ifndef __SEH_H__
#define __SEH_H__

#ifndef __seh__
#define __seh__
#endif

#if !defined(_MSC_VER)
# include <setjmp.h>

# if defined(_WIN32)
# define seh__jmpbuf_t  jmp_buf
# define seh__setjmp(x) setjmp(x)
# else
# define seh__jmpbuf_t  sigjmp_buf
# define seh__setjmp(x) sigsetjmp(x, 0)
# endif

/* The syntax */

# define __concat_in(a, b) a ## b
# define __concat(a, b) __concat_in(a, b)
# define __try                                              \
    seh__jmpbuf_t __concat(seh__env, __LINE__);             \
    seh__begin(& __concat(seh__env, __LINE__));             \
    if (seh__setjmp(__concat(seh__env, __LINE__)) == 0)

# define __except(cond) else if (cond)
# define __finally      seh__end();     
# define __leave        seh__leave()
#endif /* !defined(_MSC_VER) */

#ifdef __cplusplus
extern "C" {
#endif

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

#ifndef _MSC_VER
/* Belove functions support not be called by hand */
    
__seh__ void seh__begin(seh__jmpbuf_t* env);
__seh__ void seh__end(void);
__seh__ void seh__leave(void);
#endif

/* END OF EXTERN "C" */
#ifdef __cplusplus
}
#endif
    
#endif /* __SEH_H__ */


#if defined(SEH_IMPL)

#include <stdio.h>
#include <stdlib.h>

#ifndef seh__assert
#define seh__assert(exp, msg, ...)                          \
    do {                                                    \
        if (!(exp)) {                                       \
            fprintf(stderr,                                 \
                "Assertion failed: " #exp ".\n\t" msg,      \
                ##__VA_ARGS__);                             \
            exit(1);                                        \
        }                                                   \
    } while (0)						
#endif

#ifdef  NDEBUG
#undef  seh__assert
#define seh__assert(...) ((void)0)
#endif

#define SEH__MAXENV 128

static int seh__excode = SEH_EXCODE_NONE;
static int seh__envidx = -1;
static seh__jmpbuf_t* seh__envs[SEH__MAXENV];

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
}

int seh_get_excode(void)
{
    seh__filter(GetExceptionCode());
    return seh__excode;
}
#else

# define seh__countof(x) (sizeof(x) / sizeof((x)[0]))

# if defined(_WIN32)
static void seh__longjmp(void)
{
    seh__jmpbuf_t* env = seh__envs[seh__envidx];
    longjmp(*env, seh__excode);
}
# else
const int seh__signals[] = {
    SIGABRT, SIGFPE, SIGSEGV, SIGILL, SIGSYS, SIGBUS,
};

static void seh__longjmp(void)
{
    seh__jmpbuf_t* env = seh__envs[seh__envidx];
    siglongjmp(*env, seh__excode);
}
# endif

#if defined(_WIN32)
static LONG WINAPI seh__sighandler(EXCEPTION_POINTERS* info)
{
    seh__filter(info->ExceptionRecord->ExceptionCode);
    seh__longjmp();
    return seh__excode > SEH__EXCODE_LEAVE 
        ? EXCEPTION_CONTINUE_EXECUTION 
        : EXCEPTION_CONTINUE_SEARCH;
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

void seh__begin(seh__jmpbuf_t* env)
{
    seh__assert(env != NULL,
		"env must not be null. Are you attempting call by hand?");
    seh__assert(seh__envidx < SEH__MAXENV - 1,
		"Many __try statements are nesting too depth");
    seh__envs[++seh__envidx] = env;
}

void seh__end(void)
{
    seh__assert(seh__envidx >= 0,
		"There is no __try statements are in action. "
		"Possible have wild __finally or call seh__end() by hand.");
    if (seh__envidx >= 0)
    {
	seh__envidx--;
    }
}
#endif /* defined(_MSC_VER) */

#endif /* defined(SEH_IMPL) */
