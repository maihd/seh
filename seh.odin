package seh

import "core:testing"
import "core:mem"

import c "core:c/libc"
import posix "core:sys/posix"
import windows "core:sys/windows"

Exception_Code :: enum {
    None,
    Abort,
    Float,
    Syscall,
    Illegal_Code,
    Misalignment,
    Segment_Fault,
    Out_Of_Bounds,
    Stack_Overflow,
}

SEH_Context :: struct {
    code: Exception_Code,
}

when ODIN_OS == .Windows {
    @(private = "file")
    SEH_Context_Data :: struct {
        code: Exception_Code,
        // prev: ^SEH_Context_Data,
        saved: windows.LPTOP_LEVEL_EXCEPTION_FILTER,
        jmpbuf: c.jmp_buf,
    }

    CONTEXTS_NUM :: #config(CONTEXTS_NUM, 64)
    contexts : [CONTEXTS_NUM]SEH_Context_Data

    @(private = "file")
    curr_index : int

    begin :: proc() -> ^SEH_Context {
        assert(curr_index < len(contexts))

        ctx := &contexts[curr_index]
        ctx.code = .None
        ctx.saved = windows.SetUnhandledExceptionFilter(sighandler)

        curr_index += 1
        c.setjmp(&ctx.jmpbuf)

        return transmute(^SEH_Context)ctx
    }

    @(private = "file")
    sighandler :: proc "system" (info: ^windows.EXCEPTION_POINTERS) -> windows.LONG {
        switch info.ExceptionRecord.ExceptionCode {
        case windows.EXCEPTION_FLT_OVERFLOW:
        case windows.EXCEPTION_FLT_UNDERFLOW:
        case windows.EXCEPTION_FLT_STACK_CHECK:
        case windows.EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case windows.EXCEPTION_FLT_INEXACT_RESULT:
        case windows.EXCEPTION_FLT_DENORMAL_OPERAND:
        case windows.EXCEPTION_FLT_INVALID_OPERATION:
            throw(.Float);

        case windows.EXCEPTION_ILLEGAL_INSTRUCTION:
            throw(.Illegal_Code)

        case windows.EXCEPTION_STACK_OVERFLOW:
            throw(.Stack_Overflow);

        case windows.EXCEPTION_ACCESS_VIOLATION:
            throw(.Segment_Fault);
        
        case windows.EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            throw(.Out_Of_Bounds);

        case windows.EXCEPTION_DATATYPE_MISALIGNMENT:
            throw(.Misalignment);
        
        case:
            throw(.None);
        }

        return windows.EXCEPTION_CONTINUE_EXECUTION
    }

    end :: proc(ctx: ^SEH_Context) {
        ctx := transmute(^SEH_Context_Data)ctx

        if &contexts[curr_index - 1] == ctx {
            windows.SetUnhandledExceptionFilter(ctx.saved)
            curr_index -= 1
        }
    }

    throw :: proc "c" (code: Exception_Code) {
        curr := &contexts[curr_index]
        if curr != nil {
            curr.code = code
            c.longjmp(&curr.jmpbuf, 1)
        }
    }
} else {
    @(private = "file")
    SEH_Context_Data :: struct {
        code: Exception_Code,
        // prev: ^SEH_Context_Data,
        saved: [len(signals)]posix.sigaction_t,
        jmpbuf: c.jmp_buf,
    }

    @(private = "file")
    signals := [?]posix.Signal{ 
        .SIGABRT, .SIGFPE, .SIGSEGV, .SIGILL, .SIGSYS, .SIGBUS,
    }

    CONTEXTS_NUM :: #config(CONTEXTS_NUM, 64)
    contexts : [CONTEXTS_NUM]SEH_Context_Data

    @(private = "file")
    curr_index : int

    begin :: proc() -> ^SEH_Context {
        assert(curr_index < len(contexts))

        ctx := &contexts[curr_index]
        ctx.code = .None
        
        sa := posix.sigaction_t{
            sa_sigaction = sighandler,
            sa_flags = { .SIGINFO, .RESTART, .SA_NODEFER }
        }
        posix.sigemptyset(&sa.sa_mask)

        for i in 0..<len(signals) {
            if posix.sigaction(signals[i], &sa, &ctx.saved[i]) != .OK {
                free(ctx)
                return nil
            }
        }

        curr_index += 1
        c.setjmp(&ctx.jmpbuf)

        return transmute(^SEH_Context)ctx
    }

    end :: proc(ctx: ^SEH_Context) {
        ctx := transmute(^SEH_Context_Data)ctx

        if &contexts[curr_index - 1] == ctx {
            if curr_index > 1 {
                prev := &contexts[curr_index - 1]
                for i in 0..<len(signals) {
                    posix.sigaction(signals[i], &ctx.saved[i], &prev.saved[i])
                }
            }

            curr_index -= 1
        }
    }

    throw :: proc "c" (code: Exception_Code) {
        curr := &contexts[curr_index]
        if curr != nil {
            curr.code = code
            c.longjmp(&curr.jmpbuf, 1)
        }
    }

    @(private = "file")
    sighandler :: proc "c" (sig: posix.Signal, info: ^posix.siginfo_t, ctx: rawptr) {
        #partial switch sig {
        case .SIGBUS:
            throw(.Misalignment);

        case .SIGSYS:
            throw(.Syscall);

        case .SIGFPE:
            throw(.Float);
        
        case .SIGILL:
            throw(.Illegal_Code);

        case .SIGABRT:
            throw(.Abort);

        case .SIGSEGV:
            throw(.Segment_Fault);
        
        case:
            throw(.None);
        }
    }
}

@(test)
seh_test :: proc(t: ^testing.T) {
    ctx := begin()
    defer end(ctx)

    if ctx.code == .None {
        ptr: ^int = nil
        ptr^ = 0
    } else if ctx.code == .Segment_Fault {
        testing.expect(t, true, "")
    } else {
        testing.expect(t, false, "")
    }
}