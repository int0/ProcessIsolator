//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_process.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessList class implementation
///
//////////////////////////////////////////////////////////////////////////
#include "pi_pi.h"
#include "nt/nt_helpers.h"

NAMESPACE_PI_BEGIN

VOID ProcessList::AddProcessEntry( __in PPI_PROCESS_LIST List, __in PPI_PROCESS_ENTRY Entry )
{
    Entry->Next = List->Entry;
    Entry->Next = (PPI_PROCESS_ENTRY)InterlockedExchangePointer( (PVOID *)&List->Entry, Entry );
}

PPI_PROCESS_ENTRY ProcessList::RemoveProcessEntry( __in PPI_PROCESS_LIST List )
{
    if( NULL == List->Entry )
        return NULL;

    return (PPI_PROCESS_ENTRY)InterlockedExchangePointer( (PVOID *)&List->Entry, List->Entry->Next );
}

PPI_PROCESS_ENTRY ProcessList::GetProcessEntry( __in PPI_PROCESS_LIST List, __in HANDLE ProcessId )
{
    PPI_PROCESS_ENTRY Entry = List->Entry;
    
    while( NULL != Entry )
    {
        if( Entry->ProcessId == ProcessId )
            return Entry;
        
        Entry = Entry->Next;
    }

    return NULL;
}

NTSTATUS ProcessList::TerminateIsolatedProcess( __in PPI_PROCESS_ENTRY ProcessEntry )
{
    return ObTerminateProcess( ProcessEntry->Process );
}

/// <summary>
/// Terminates all isolated processes.
/// </summary>
/// <param name="List">The list of isolated processes.</param>
/// <returns>Returns STATUS_SUCCESS if ALL processes were terminated otherwise it returns STATUS_UNSUCCESSFUL.</returns>
NTSTATUS ProcessList::TerminateAllIsolatedProcesses( __in PPI_PROCESS_LIST List )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPI_PROCESS_ENTRY Entry = List->Entry;
    
    while( NULL != Entry )
    {
        //
        // kill process
        if( !NT_SUCCESS( Status = ProcessList::TerminateIsolatedProcess( Entry ) ) )
            Status = STATUS_UNSUCCESSFUL;
        //
        // get next entry
        Entry = Entry->Next;
    }

    return Status;
}

BOOLEAN ProcessList::IsProcessBusy( __in PPI_PROCESS_ENTRY Entry )
{
    return ( 0 != Entry->ThreadCount );
}

BOOLEAN ProcessList::ListHaveWaitingThreads( __in PPI_PROCESS_LIST List )
{
    PPI_PROCESS_ENTRY Entry = List->Entry;

    while( NULL != Entry )
    {
        if( IsProcessBusy( Entry ) )
            return TRUE;
        
        Entry = Entry->Next;
    }
    
    return FALSE;
}

NAMESPACE_PI_END