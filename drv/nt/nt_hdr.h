//////////////////////////////////////////////////////////////////////////
///
/// File:           nt_hdr.h
/// Author:         Volodymyr Pikhur
/// Description:    Windows kernel structures header file
///
//////////////////////////////////////////////////////////////////////////
#pragma  once
#include <ntifs.h>

typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
    PULONG_PTR Base;
    PULONG Count;
    SIZE_T Limit;
    PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

//
// Returns just process image name not full path also not documented but exported by nt kernel
//
extern "C" NTKERNELAPI PCHAR NTAPI PsGetProcessImageFileName( __in PEPROCESS Process );
extern "C" NTKERNELAPI NTSTATUS NTAPI ZwQueryInformationProcess(
    __in       HANDLE ProcessHandle,
    __in       PROCESSINFOCLASS ProcessInformationClass,
    __out      PVOID ProcessInformation,
    __in       ULONG ProcessInformationLength,
    __out_opt  PULONG ReturnLength
    );
