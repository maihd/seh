# Introduction
Simple and cross-compiler [Structured Exception Handling](https://docs.microsoft.com/en-us/cpp/cpp/structured-exception-handling-c-cpp?view=vs-2019) for C/C++

## Versions
1. seh.h: Exception handler that fully listenning on system signal and custom
2. seh_lite.h: Exception handler without listenning on system signal

## Examples 
seh.h:
```C
seh_t seh;
seh_try (seh)
{
    int* ptr = NULL;
    *ptr = 0; /* Throw exception here */
}
seh_catch (seh_get() == SEH_SEGFAULT)
{
    fprintf(stderr, "Segment fault exception has been thrown\n");
}
seh_finally (seh)
{
    printf("Finally of try/catch\n");
}
```

seh_lite.h
```C
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
```
