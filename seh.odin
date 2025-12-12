package seh

import "core:testing"
import "core:mem"
import "core:fmt"

import c "core:c/libc"
import posix "core:sys/posix"

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
    #panic("SEH on Windows does not implemented yet!")
} else {
    @(private = "file")
    SEH_Context_Data :: struct {
        code: Exception_Code,
        prev: ^SEH_Context_Data,
        saved: [^]posix.sigaction_t,
        jmpbuf: c.jmp_buf,
    }

    @(private = "file")
    signals := []posix.Signal{ 
        .SIGABRT, .SIGFPE, .SIGSEGV, .SIGILL, .SIGSYS, .SIGBUS,
    }

    @(private = "file")
    curr : ^SEH_Context_Data

    begin :: proc() -> ^SEH_Context {
        ctx := cast(^SEH_Context_Data)make([^]u8, size_of(SEH_Context_Data) + size_of(posix.sigaction_t) * len(signals))

        ctx.prev = nil
        ctx.code = .None
        ctx.saved = cast([^]posix.sigaction_t)mem.ptr_offset(cast([^]u8)ctx, size_of(SEH_Context_Data))
        
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

        ctx.prev = curr
        curr = ctx

        c.setjmp(&ctx.jmpbuf)

        return transmute(^SEH_Context)ctx
    }

    end :: proc(ctx: ^SEH_Context) {
        ctx := transmute(^SEH_Context_Data)ctx

        if curr == ctx {
            for i in 0..<len(signals) {
                if posix.sigaction(signals[i], &ctx.saved[i], &ctx.prev.saved[i]) != .OK {
                    free(ctx)
                    return
                }
            }

            curr = ctx.prev
            free(ctx)
        }
    }

    throw :: proc "c" (code: Exception_Code) {
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