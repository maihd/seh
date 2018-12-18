# Introduction
Simple and cross-compiler 'Structured Exception Handling' for C/C++

## Examples 
seh.h:
```C
if (seh_init() != SEH_INIT_SUCCESS)
{
    /* init failed */
}
...
__try
{
    int* ptr = NULL;
    *ptr = 0;
}
__except (seh_get_excode() == SEH_EXCODE_SEGFAULT)
{
    /* Reach here */
}
__except (seh_get_excode() > SEH_EXCODE_LEAVE)
{
    /* Never reached */
}
__finally /* Must has __finally */
{
    /* @TODO: finally code after __try/__except */
}

...
seh_quit(); /* release seh's memory usage */
```

seh_ex.h
```C
seh_ctx_t ctx;
seh_try (ctx)
{
    ...
    seh_throw(1);
}
seh_catch (seh_get() == 1)
{
    /* Reach here */
}
seh_catch (seh_get() == 2)
{
    /* Never reached */
}
seh_finally (ctx) /* Must has __finally */
{
    /* @TODO: finally code after __try/__except */
}
```