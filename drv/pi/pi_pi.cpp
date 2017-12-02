//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_pi.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator implementation
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>
#include "pi_process.h"
#include "pi_pi.h"
#include "trap/pi_trap.h"
#include "msr/pi_msr.h"
#include "cpu/pi_cpu.h"

NAMESPACE_PI_BEGIN

ProcessIsolator::ProcessIsolator( __in PPI_DEVICE_EXTENSION DevExt )
{
    _DevExt = DevExt;
    _Log = NULL;
    _Mem = NULL;
    _LogHandle = NULL;
    _PrintBuffer = NULL;
    _PrintBufferSize = 0;
    _ProcessList = NULL;
    _List = &g_PIProcessList;
}

ProcessIsolator::~ProcessIsolator()
{
    PI_Stop();
    Destroy();
}

VOID ProcessIsolator::Destroy()
{
    if( NULL != _ProcessList )
    {
        delete _ProcessList;
        _ProcessList = NULL;
    }
    //
    // close log handle and class
    if( NULL != _Log )
    {
        if( NULL != _LogHandle )
        {
            _Log->Close( _LogHandle );
            _LogHandle = NULL;
        }
        delete _Log;
        _Log = NULL;
    }

    //
    // Free log buffer
    if( NULL != _PrintBuffer )
    {
        _Mem->Free( _PrintBuffer );
        _PrintBufferSize = 0;
        _PrintBuffer = NULL;
    }

    //
    // destroy memory manager
    if( NULL != _Mem )
    {
        delete _Mem;
        _Mem = NULL;
    }
}

BOOLEAN ProcessIsolator::PI_Init()
{
    //
    // set printing buffer to 4KB
    _PrintBufferSize = 4096;

    //
    // create memory manager class
    _Mem = new PI::Memory();
    
    //
    // initialize memory manager
    if( NULL == _Mem || !_Mem->Init() )
        goto _Cleanup;
    
    //
    // create process list manager
    if( NULL == ( _ProcessList = new PI::ProcessList( _Mem ) ) )
        goto _Cleanup;

    //
    // allocate buffer for log
    if( NULL == (_PrintBuffer = _Mem->Alloc( _PrintBufferSize ) ) )
        goto _Cleanup;

    //
    // create log class
    if( NULL == (_Log = new LogFile( (PCHAR)_PrintBuffer, _PrintBufferSize ) ) )
        goto _Cleanup;

    //
    // create global log file on c:\pi_global.log
    if( NULL == ( _LogHandle = _Log->Create( L"C:", L"pi_global", 0 ) ) )
        goto _Cleanup;

    _Log->Print( _LogHandle, "ProcessIsolator initialization successful\n" );
    return TRUE;

_Cleanup:
    
    Destroy();
    
    return FALSE;
}

VOID ProcessIsolator::PI_SetSysenterCallback( __in_opt ULONG CpuNumber, __in_opt PVOID Context )
{
    UNREFERENCED_PARAMETER( CpuNumber );
    UNREFERENCED_PARAMETER( Context );

    if( NULL != Context )
        MsrManager::SetSysEnterHandler( Context );
}

NTSTATUS ProcessIsolator::PI_Start()
{
    //
    // Hook SYSENTER by modifying MSR
    //
#ifdef _AMD64_
    NT_KiSystemCall64 = MsrManager::GetSysEnterHandlerAddress();
    CPUManager::RunCallbackOnEveryCpu( PI_SetSysenterCallback, PI_KiSystemCall64 );
#else
#endif

    return STATUS_SUCCESS;
}

NTSTATUS ProcessIsolator::PI_Stop()
{
    PCHAR ErrorMsg;
    NTSTATUS Status;

    //
    // 0) Drop all client connections
    // 1) Kill all processes
    // 2) Make sure 0 threads awaits for server    
    // 3) Unregister all processes
    // 4) Unhook MSR

    PI_DropAllConnections( _List );

    if( !NT_SUCCESS( Status = _ProcessList->TerminateAllIsolatedProcesses( _List ) ) )
    {
        ErrorMsg = "Unable to terminate all isolated processes";
        goto _ReportError;
    }

    if( _ProcessList->ListHaveWaitingThreads( _List ) )
    {
        ErrorMsg = "There are waiting threads in SysEnter handler";
        Status = STATUS_UNSUCCESSFUL;
        goto _ReportError;
    }
    
    if( !NT_SUCCESS( Status = PI_UnregisterAllProcesses( _List ) ) )
    {
        ErrorMsg = "Unable to unregister all processes";
        goto _ReportError;
    }

#ifdef _AMD64_
    if( NULL != NT_KiSystemCall64 )
    {
        CPUManager::RunCallbackOnEveryCpu( PI_SetSysenterCallback, NT_KiSystemCall64 );
        NT_KiSystemCall64 = NULL;
    }
#endif

    return STATUS_SUCCESS;

_ReportError:
    
    PILog.Print( PILogHandle, "[ERROR][%x]: Can't stop ProcessIsolator: %s", Status, ErrorMsg );

    return Status;
}

NAMESPACE_PI_END