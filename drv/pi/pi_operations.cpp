//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_operations.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator IOCTL operations implementation
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>
#include "pi_pi.h"
#include "pi_process.h"
#include "nt/nt_helpers.h"

NAMESPACE_PI_BEGIN

NTSTATUS ProcessIsolator::PI_PerformMemoryOperation( PI_OPERATION_TYPE OperationType, HANDLE ProcessId, PVOID TargetAddress, PVOID Buffer, ULONG BuffserSize )
{
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( ProcessId );

    if( MmHighestUserAddress < TargetAddress &&
        MmIsAddressValid( TargetAddress ) )
    {
        PMDL Mdl;
        PVOID From, To, Mem;

        if( NULL != ( Mdl = IoAllocateMdl( TargetAddress, BuffserSize, FALSE, FALSE, NULL ) ) )
        {
            __try
            {
                LOCK_OPERATION LockOp = ( ReadMemory == OperationType ) ? IoReadAccess : IoWriteAccess;
                
                MmProbeAndLockPages( Mdl, KernelMode, LockOp );

                if( NULL != ( Mem = MmMapLockedPagesSpecifyCache( Mdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority ) ) )
                {
                    if( ReadMemory == OperationType )
                    {
                        From = Mem;
                        To = Buffer;
                    }
                    else
                    {
                        From = Buffer;
                        To = Mem;
                    }

                    RtlCopyMemory( To, From, BuffserSize );
                    MmUnmapLockedPages( Mem, Mdl );
                }
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                Status = GetExceptionCode();
            }

            IoFreeMdl( Mdl );
        }

    }
    else
    {
        //     PEPROCESS Process;
        //     KAPC_STATE ApcState;
        //     if( NTSTATUS( Status = PsLookupProcessByProcessId( ProcessId, &Process ) ) )
        //     {
        //         KeStackAttachProcess( Process, &ApcState );
        // 
        //         //MmGetVirtualForPhysical(  )
        // 
        //         if( OperationType == MemoryRead )
        //         {
        //             //RtlCopyMemory( Buffer,  )
        //             //Zw
        //         }
        //         else
        //         {
        // 
        //         }
        // 
        //         KeUnstackDetachProcess( &ApcState );
        //         ObDereferenceObject ( Process );
        //     }
        //     
        Status = STATUS_ACCESS_VIOLATION;
    }

    return Status;
}

NTSTATUS ProcessIsolator::PI_RegisterProcess( __in PPI_PROCESS_LIST List, __in HANDLE ProcessId )
{
    NTSTATUS Status;
    PPI_PROCESS_ENTRY Entry;

    if( NULL == ( Entry = _ProcessList->AllocateProcessEntry() ) )
        return STATUS_INSUFFICIENT_RESOURCES;

    if( NT_SUCCESS( Status = PsLookupProcessByProcessId( ProcessId, &Entry->Process ) ) )
    {
        Entry->Client = (PVOID)new PI::LpcClient( _Mem );        
        
        if( NULL == Entry->Client )
            goto _Cleanup;

        if( PIClient(Entry)->Connect( L"\\ProcessIsolator" ) )
        {            
            Entry->ProcessId = ProcessId;
            _ProcessList->AddProcessEntry( List, Entry );
        }
        else
        {
            Status = STATUS_SERVER_UNAVAILABLE;
        }
    }

_Cleanup:

    if( !NT_SUCCESS(Status) )
    {
        PILog.Print( PILogHandle, "ERROR[%x] Unable to register process with id %d", Status, (ULONG)ProcessId );

        if( NULL != Entry->Process )
            ObDereferenceObject( Entry->Process );       

        TerminateProcessById( ProcessId );

        if( NULL != Entry->Client )
            delete PIClient(Entry);
        
        _ProcessList->FreeProcessEntry( Entry );        
    }
    else
    {
        PILog.Print( PILogHandle, "New process registered: %d", (ULONG)ProcessId );
    }

    return Status;
}

NTSTATUS ProcessIsolator::PI_UnregisterProcess( __in PPI_PROCESS_LIST List, __in HANDLE ProcessId )
{
    PPI_PROCESS_ENTRY Prev = NULL;
    PPI_PROCESS_ENTRY Entry = List->Entry;
    
    if( NULL == Entry )
        return STATUS_NO_MORE_ENTRIES;

    while ( NULL != Entry )
    {
        if( Entry->ProcessId == ProcessId )
        {
            //
            // Delete LPC client
            if( NULL != Entry->Client )
                delete PIClient(Entry);
            //
            // Remove entry from list
            if( NULL == Prev )
                InterlockedExchangePointer( (PVOID *)&List->Entry, Entry->Next );
            else
                InterlockedExchangePointer( (PVOID *)&Prev->Next, Entry->Next );
            //
            // Dereference process object
            ObDereferenceObject( Entry->Process );
            //
            // Free process list entry
            _ProcessList->FreeProcessEntry( Entry );

            return STATUS_SUCCESS;
        }
        
        Prev = Entry;
        Entry = Entry->Next;
    }
    return STATUS_NOT_FOUND;
}

VOID ProcessIsolator::PI_DropAllConnections( __in PPI_PROCESS_LIST List )
{
    PPI_PROCESS_ENTRY Entry = List->Entry;

    while( NULL != Entry )
    {
        PIClient(Entry)->CloseConnection();
        //
        // get next entry
        Entry = Entry->Next;
    }
}

NTSTATUS ProcessIsolator::PI_UnregisterAllProcesses( __in PPI_PROCESS_LIST List )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPI_PROCESS_ENTRY Entry = List->Entry;

    while( NULL != Entry )
    {
        //
        // unregister process
        if( !NT_SUCCESS( Status = PI_UnregisterProcess( List, Entry->ProcessId ) ) )
            Status = STATUS_UNSUCCESSFUL;
        //
        // get next entry
        Entry = Entry->Next;
    }

    return Status;
}

NTSTATUS ProcessIsolator::PI_ProcessIoctlOperation( __in PPI_OPERATION_REQUEST Operation )
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    switch( Operation->Type )
    {
    case PI::StartProcessIsolator:
        {
            Status = PI_Start();
        }break;
    case PI::StopProcessIsolator:
        {
            Status = PI_Stop();
        }break;
    case PI::RegisterProcess:
        {
            Status = PI_RegisterProcess( _List, Operation->ProcessId );
        }break;
    case PI::UnregisterProcess:
        {
            Status = PI_UnregisterProcess( _List, Operation->ProcessId );
        }break;
    case PI::TerminateProcess:
        {
            Status = TerminateProcessById( Operation->ProcessId );
        }break;
    case PI::ReadMemory:
    case PI::WriteMemory:
        {
            __try
            {
                //
                // Probe I/O user buffer
                if( ReadMemory == Operation->Type )
                    ProbeForWrite( Operation->Memory.Buffer, Operation->Memory.BufferSize, sizeof(ULONG) );
                else
                    ProbeForRead( Operation->Memory.Buffer, Operation->Memory.BufferSize, sizeof(ULONG) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                Status = GetExceptionCode();
            }
                
            Status = PI_PerformMemoryOperation( Operation->Type, 
                Operation->ProcessId, 
                Operation->Memory.Address, 
                Operation->Memory.Buffer, 
                Operation->Memory.BufferSize );
        }break;
    }

    return Status;
}

NAMESPACE_PI_END