//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_process.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessList header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <pi/pi.h>

NAMESPACE_PI_BEGIN

typedef struct _PI_PROCESS_ENTRY
{
    struct _PI_PROCESS_ENTRY *Next;
    HANDLE ProcessId;
    PEPROCESS Process;
    /// <summary>
    /// Holds threads count entered into system service handler
    /// </summary>
    LONG ThreadCount;
    /// <summary>
    /// The LPC Client class
    /// </summary>
    PVOID Client;    
} PI_PROCESS_ENTRY, *PPI_PROCESS_ENTRY;

typedef struct _PI_PROCESS_LIST 
{
    PPI_PROCESS_ENTRY Entry;
} PI_PROCESS_LIST, *PPI_PROCESS_LIST;

class ProcessList
{
private:
    Memory *_Mem;
public:
    ProcessList( __in Memory *Mem )
    {
        _Mem = Mem;
    }
    PPI_PROCESS_ENTRY AllocateProcessEntry()
    {
        PPI_PROCESS_ENTRY Entry = (PPI_PROCESS_ENTRY)_Mem->Alloc( sizeof(PI_PROCESS_ENTRY) );
        RtlZeroMemory( Entry, sizeof(PI_PROCESS_ENTRY) );
        return Entry;
    }
    VOID FreeProcessEntry( __in PPI_PROCESS_ENTRY Entry )
    {
        _Mem->Free( Entry );
    }
    NTSTATUS TerminateAllIsolatedProcesses( __in PPI_PROCESS_LIST List );
    static VOID AddProcessEntry( __in PPI_PROCESS_LIST List, __in PPI_PROCESS_ENTRY Entry );
    static PPI_PROCESS_ENTRY RemoveProcessEntry( __in PPI_PROCESS_LIST List );
    static PPI_PROCESS_ENTRY GetProcessEntry( __in PPI_PROCESS_LIST List, __in HANDLE ProcessId );
    static NTSTATUS TerminateIsolatedProcess( __in PPI_PROCESS_ENTRY ProcessEntry );
    static BOOLEAN IsProcessBusy( __in PPI_PROCESS_ENTRY Entry );
    static BOOLEAN ListHaveWaitingThreads( __in PPI_PROCESS_LIST List );
};

NAMESPACE_PI_END