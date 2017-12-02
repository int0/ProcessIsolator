//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_types.h
/// Author:         Volodymyr Pikhur
/// Description:    
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "pi.h"

NAMESPACE_PI_BEGIN

#ifdef _AMD64_
#define REG_CONTEXT REG64_CONTEXT
#elif _X86_
#define REG_CONTEXT REG32_CONTEXT
#endif

typedef struct _REG32_CONTEXT
{
    ULONG Eax;
    ULONG Ecx;
    ULONG Edx;
    ULONG Ebx;
    ULONG Esp;
    ULONG Ebp;
    ULONG Esi;
    ULONG Edi;
} REG32_CONTEXT, *PREG32_CONTEXT;

typedef struct _REG64_CONTEXT
{
    ULONG64	Rax;
    ULONG64	Rcx;
    ULONG64	Rdx;
    ULONG64	Rbx;
    ULONG64	Rsp;
    ULONG64	Rbp;
    ULONG64	Rsi;
    ULONG64	Rdi;
    ULONG64	R8;
    ULONG64	R9;
    ULONG64	R10;
    ULONG64	R11;
    ULONG64	R12;
    ULONG64	R13;
    ULONG64	R14;
    ULONG64	R15;
} REG64_CONTEXT, *PREG64_CONTEXT;

typedef struct _PI_SYSENTER_REPLY
{
    BOOLEAN Execute;        ///< Indicates if system call should be executed
    NTSTATUS Status;        ///< If Execute is FALSE this Status will be returned
} PI_SYSENTER_REPLY, *PPI_SYSENTER_REPLY;

typedef struct _PI_SYSENTER_CONTEXT
{
    ULONG ServiceNumber;            ///< Number of system service
    ULONG_PTR SysExitAddress;       ///< Return address in user-mode after sysenter instruction
    UCHAR ArgsCount;                ///< Number of system service arguments 
    ULONG_PTR Arguments[32];        ///< System service arguments
} PI_SYSENTER_CONTEXT, *PPI_SYSENTER_CONTEXT;

typedef struct _PI_SYSENTER_REQUEST
{
    ULONG ServiceNumber;
    PVOID RegContext;    
} PI_SYSENTER_REQUEST, *PPI_SYSENTER_REQUEST;

typedef struct _PI_CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} PI_CLIENT_ID, *PPI_CLIENT_ID;

NAMESPACE_PI_END