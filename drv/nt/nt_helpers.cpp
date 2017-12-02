//////////////////////////////////////////////////////////////////////////
///
/// File:           nt_helpers.cpp
/// Author:         Volodymyr Pikhur
/// Description:    Helpers for Windows kernel implementation
///
//////////////////////////////////////////////////////////////////////////
#include "nt_hdr.h"
#include <intrin.h>

PUCHAR FindBytes( __in_bcount(Size) PVOID Address, __in ULONG Size, __in_bcount(BytesCount) PUCHAR Bytes, __in ULONG BytesCount )
{
    ULONG i;

    for ( i = 0; i < Size - BytesCount; i++ )
    {
        if( 0 == memcmp( &((PUCHAR)Address)[i], Bytes, BytesCount ) )
            return &((PUCHAR)Address)[i];
    }

    return NULL;
}

PVOID GetKeServiceDescriptorTable( __in PVOID SystemCallHandler )
{
    static PVOID pKeServiceDescriptorTable = NULL;

    UNREFERENCED_PARAMETER( SystemCallHandler );

#ifdef _AMD64_
    if( NULL != pKeServiceDescriptorTable )
        return pKeServiceDescriptorTable;

    if( SystemCallHandler == NULL )
        return NULL;
         
    static UCHAR KiSystemServiceStartBytes[] = 
    { 0x8B, 0xF8, 0xC1, 0xEF, 0x07, 0x83, 0xE7, 0x20, 0x25, 0xFF, 0x0F, 0x00, 0x00, 0x4C, 0x8D };

    pKeServiceDescriptorTable =
        FindBytes( SystemCallHandler, 0x200, KiSystemServiceStartBytes, sizeof(KiSystemServiceStartBytes) );

    if( pKeServiceDescriptorTable )
    {
        PUCHAR pKiSystemServiceRepeat = 
            (PUCHAR)pKeServiceDescriptorTable + sizeof(KiSystemServiceStartBytes) + 5;

        pKeServiceDescriptorTable = 
            (PVOID)( pKiSystemServiceRepeat + *(PULONG)( pKiSystemServiceRepeat + 3 ) + 7 );
        
        return pKeServiceDescriptorTable;
    }
#endif

    return pKeServiceDescriptorTable;
}

PVOID GetSystemServiceAddressFromIndex( __in ULONG SvcIndex, __in PVOID SystemCallHandler )
{
    PKSERVICE_TABLE_DESCRIPTOR SvcTable;

    if( NULL == ( SvcTable = (PKSERVICE_TABLE_DESCRIPTOR)GetKeServiceDescriptorTable( SystemCallHandler ) ) )
        return NULL;

    ULONG ServiceIndex = SvcIndex & 0xFFF;
    ULONG TableIndex = ( SvcIndex >> 12 ) & 0x1;

    if( ServiceIndex >= SvcTable[TableIndex].Limit )
        return NULL;
    
#ifdef _AMD64_
        PLONG Base = (PLONG)SvcTable[TableIndex].Base;
        LONG64 Delta = Base[ServiceIndex];
        return (PVOID)((PUCHAR)Base + ( Delta >> 4 ) );
#elif _X86_
    return (PVOID)SvcTable[TableIndex].Base[ServiceIndex];
#endif

}

NTSTATUS ObTerminateProcess( __in PEPROCESS Process )
{
    NTSTATUS Status;
    HANDLE ProcessHandle;

    Status = ObOpenObjectByPointer( Process, 0, 0, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &ProcessHandle );

    if( NT_SUCCESS(Status) )
    {
        Status = ZwTerminateProcess( ProcessHandle, STATUS_SUCCESS );

        if( NT_SUCCESS(Status) || STATUS_PROCESS_IS_TERMINATING == Status )
        {
            Status = STATUS_SUCCESS;
            LARGE_INTEGER Timeout;
            Timeout.QuadPart = -5 * 10 * 1000 * 1000;       // wait 5 sec
            if( STATUS_SUCCESS != ( Status = ZwWaitForSingleObject( ProcessHandle, FALSE, &Timeout ) ) )
            {
                DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__"::ZwWaitForSingleObject = %x PID: %d\n", Status, (ULONG)PsGetProcessId( Process ) );
            }            
        }
        else
        {
#ifdef DBG
            DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__"::ZwTerminateProcess = %x\n", Status );
#endif
        }
        ZwClose( ProcessHandle );
    }
    else
    {
#ifdef DBG
        DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__"::ObOpenObjectByPointer = %x", Status );
#endif
    }

    return Status;
}

NTSTATUS TerminateProcessById( __in HANDLE ProcessId )
{
    NTSTATUS Status;
    PEPROCESS Process;

    if( NT_SUCCESS( Status = PsLookupProcessByProcessId( ProcessId, &Process ) ) )
    {
        Status = ObTerminateProcess( Process );
        ObDereferenceObject ( Process );
    }

    return Status;
}

NTSTATUS PsGetProcessImagePath( __in PEPROCESS Process, __in PUNICODE_STRING ImagePath, __inout PULONG Size )
{
    NTSTATUS Status;
    HANDLE ProcessHandle;

    Status = ObOpenObjectByPointer( Process, 0, 0, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &ProcessHandle );

    if( NT_SUCCESS(Status) )
    {
        Status = ZwQueryInformationProcess( ProcessHandle, ProcessImageFileName, ImagePath, *Size, Size );
        ZwClose( ProcessHandle );
    }

    return Status;
}