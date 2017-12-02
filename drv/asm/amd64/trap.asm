public PI_KiSystemCall64
public NT_KiSystemCall64

PI_PROCESS_ENTRY struct
    Next				dq		?
    ProcessId			dq		?
    Process				dq		?
PI_PROCESS_ENTRY ends

PI_PROCESS_LIST struct
    Entry				dq		?
PI_PROCESS_LIST ends

extern PsGetCurrentProcessId:proc
extern PI_KiSystemCall64_Handler:proc
extern g_PIProcessList:PI_PROCESS_LIST

.data

NT_KiSystemCall64 dq 0

.code

PI_KiSystemCall64 proc

    swapgs											;;; Swap GS base from TEB to KPCR

    mov   gs:[10h], rsp								;;; Save user stack ptr
    mov   rsp, gs:[1A8h]							;;; Get kernel stack ptr
    
    pushfq

    push	rax
    push	rcx
    push	rdx

    call	PsGetCurrentProcessId

    mov		rdx, rax
    lea		rcx, g_PIProcessList
    mov		rcx, [rcx+PI_PROCESS_LIST.Entry]
    xor		rax, rax

_ContinueSearch:	
    cmp		rax, rcx	
    jz		_NoProcessList
    
    cmp		rdx, [rcx+PI_PROCESS_ENTRY.ProcessId]
    mov		rcx, [rcx+PI_PROCESS_ENTRY.Next]
    jnz		_ContinueSearch
    
    inc		rax

_NoProcessList:
    pop		rdx
    pop		rcx
    pop		rax
    jz		_Continue
    
    ;;;
    ;;; KTRAP_FRAME			190h
    ;;; REG64_CONTEXT		8 * 16
    ;;;	Rbp

    push		rbp
    sub			rsp, 190h + 8 * 16						;;; allocate space for REG64_CONTEXT and KTRAP_FRAME stucture	
    lea			rbp, [rsp + 190h]
    
    ;;;
    ;;;		SAVE REG64_CONTEXT
    ;;;	
    mov			[rbp + 8 *  0], rax
    mov			[rbp + 8 *  1], rcx
    mov			[rbp + 8 *  2], rdx
    mov			[rbp + 8 *  3], rbx
    mov			rax, gs:[10h]							;;; save user rsp
    mov			[rbp + 8 *  4], rax
    mov			rax, [rbp + 8 * 16]						;;; save original rbp
    mov			[rbp + 8 *  5], rax
    mov			[rbp + 8 *  6], rsi
    mov			[rbp + 8 *  7], rdi
    mov			[rbp + 8 *  8],  r8
    mov			[rbp + 8 *  9],  r9
    mov			[rbp + 8 * 10], r10
    mov			[rbp + 8 * 11], r11
    mov			[rbp + 8 * 12], r12
    mov			[rbp + 8 * 13], r13
    mov			[rbp + 8 * 14], r14
    mov			[rbp + 8 * 15], r15

    mov			[rsp+140h], rbx							;;; KTRAP_FRAME.Rbx
    mov			[rsp+148h], rdi							;;; KTRAP_FRAME.Rdi
    mov			[rsp+150h], rsi							;;; KTRAP_FRAME.Rsi
    mov			[rsp+158h], rax							;;; KTRAP_FRAME.Rbp
    xor			rax, rax
    mov			[rsp+160h], rax							;;; KTRAP_FRAME.ErrorCode	= 0 ( not important )
    mov			[rsp+168h], rcx							;;; KTRAP_FRAME.Rip			user mode ret address
    mov			rax, 33h
    mov			[rsp+170h], rax							;;; KTRAP_FRAME.SegCs		= 33h
    mov			[rsp+178h], r11							;;; KTRAP_FRAME.EFlags		save user mode EFlags
    mov			rax, gs:[10h]							;;; KPCR.UserRsp
    mov			[rsp+180h], rax							;;; KTRAP_FRAME.Rsp			user mode Rsp
    mov			rax, 2Bh
    mov			[rsp+188h], rax							;;; KTRAP_FRAME.SegSs		= 2Bh
    mov			rbx, gs:[188h]							;;; Get KTHREAD from KPCR
    prefetchw	byte ptr [rbx+1D8h]						;;; KTHREAD.TrapFrame
    mov			[rbx+1E0h], r10							;;; KTHREAD.FirstArgument
    mov			rax, [rbp]								;;; REG64_CONTEXT.Rax
    mov			[rbx+1F8h], eax							;;; KTHREAD.SystemCallNumber
    mov			[rbx+1D8h], rsp							;;; KTHREAD.TrapFrame =  KTRAP_FRAME
    mov			byte ptr [rsp+2Bh], 2					;;; KTRAP_FRAME.ExceptionActive
    mov			word ptr [rsp+100h], 0					;;; KTRAP_FRAME.Dr7
    stmxcsr		dword ptr [rsp + 2Ch]					;;; Save user MxCsr
    ldmxcsr		dword ptr gs:[180h]						;;; Load kernel MxCsr	
    
    lea			rcx, [rbp]								;;; get REG64_CONTEXT	
    
    sti													;;; enable interrupts
    call		PI_KiSystemCall64_Handler				;;; call our service handler send context to server
    cli													;;; disable interrupts	
    
    test		al, al									;;; test if we return to user mode or continue execution
    
    ldmxcsr		dword ptr [rsp + 2Ch]					;;; Restore user MxCsr	
    
    mov			rax, [rsp + 180h]						;;; KTRAP_FRAME.Rsp
    mov			gs:[10h], rax							;;; restore user Rsp

    mov			rax, [rbp + 8 * 0]
    mov			rcx, [rbp + 8 * 1]
    mov			rdx, [rbp + 8 * 2]
    mov			rbx, [rbp + 8 * 3]
    mov			rsi, [rbp + 8 * 6]
    mov			rdi, [rbp + 8 * 7]
    mov			 r8, [rbp + 8 * 8]
    mov			 r9, [rbp + 8 * 9]
    mov			r10, [rbp + 8 * 10]
    mov			r11, [rbp + 8 * 11]
    mov			r12, [rbp + 8 * 12]
    mov			r13, [rbp + 8 * 13]
    mov			r14, [rbp + 8 * 14]
    mov			r15, [rbp + 8 * 15]
    
    mov			rbp, [rbp + 8 * 5]

    jz			_Return									;;; Return to user mode
    
    lea			rsp, [rsp + 190h + 8 * 17]

_Continue:
    
    popfq												;;; Restore RFLAGS
    mov rsp, gs:[10h]									;;; Restore user stack
    swapgs												;;; Swap back KPCR to TEB
    jmp NT_KiSystemCall64								;;; Jump to original handler

_Return:

    mov r11,	[rsp+178h]								;;; Restore user RFLAGS
    mov rcx,	[rsp+168h]								;;; Restore user Rip

    xor     edx, edx
    pxor    xmm0, xmm0
    pxor    xmm1, xmm1
    pxor    xmm2, xmm2
    pxor    xmm3, xmm3
    pxor    xmm4, xmm4
    pxor    xmm5, xmm5

    lea			rsp, [rsp + 190h + 8 * 17]

    popfq												;;; Restore RFLAGS
    mov rsp, gs:[10h]									;;; Restore user stack
    swapgs												;;; Swap back KPCR to TEB
    sysretq												;;; Exit to user mode

PI_KiSystemCall64 endp

end