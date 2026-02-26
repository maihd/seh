# Introduction
Simple and cross-compiler [Structured Exception Handling](https://docs.microsoft.com/en-us/cpp/cpp/structured-exception-handling-c-cpp?view=vs-2019) for C/C++.

## Versions
1. seh.h: Exception handler that fully listenning on system signal and custom
2. seh_lite.h: Exception handler without listenning on system signal

## Usages
- Safe native plugins
- Native code as scripting language. 
- Safe hot reloading. Example from Mai game [NeonShooter](https://github.com/maihd/neonshooter/tree/odin-raylib)

## Tested plaforms
- Windows
- Linux
- MacOS

## Examples 
seh.h:
```C
seh_try (seh) // `seh` is name of SEH context variable
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

## Bindings
This library rely on setjmp and native signal handling (SEH on Windows). The API was designed to make usage code easy to tracks which code will handle exceptions and have special meaning.
But it does not mean we cannot use the library without theses macros. Below are simple code that does not use macros:
```C
seh_t* seh = seh_begin(&seh);
if (setjmp(seh->jmpbuf) == 0)
{
    int* ptr = NULL;
    *ptr = 0; /* Throw exception here */
}
else if (seh->value == SEH_SEGFAULT)
{
    fprintf(stderr, "Segment fault exception has been thrown\n");
}
else
{
    fprintf(stderr, "An exception is occurred\n");
}

seh_end(seh);
printf("Finally of try/catch\n");
```
Now the code are clearly have no used of macros, just functions and statements, we can create bindings now, evenly rewritten on other languages. Let see [Odin port](/seh.odin)

## Acknowledges
I have firstly read about SEH when exploring native language as scripting from Molecular Musings blog: https://blog.molecular-matters.com/2014/05/10/using-runtime-compiled-c-code-as-a-scripting-language-under-the-hood/

## License
UNLICENSE! You can use, rewrite, own freely with this code.