//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_pi.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "pi_process.h"

namespace PI
{
    class ProcessIsolator;
}

typedef struct _PI_DEVICE_EXTENSION
{
    UNICODE_STRING ServicePath;             ///< Must be freed
    UNICODE_STRING DeviceName;
    UNICODE_STRING SymlinkName;
    PDRIVER_OBJECT DriverObject;
    PDEVICE_OBJECT DeviceObject;
    RTL_OSVERSIONINFOW OsVerInfo;
    PI::ProcessIsolator *Pi;
} PI_DEVICE_EXTENSION, *PPI_DEVICE_EXTENSION;

NAMESPACE_PI_BEGIN
class ProcessIsolator
{
private:
    //////////////////////////////////////////////////////////////////////////
    // Private members
    // 
    PI::Memory *_Mem;
    PI::LogFile *_Log;
    PI::ProcessList *_ProcessList;
    PPI_PROCESS_LIST _List;
    HANDLE _LogHandle;    
    PVOID _PrintBuffer;
    ULONG _PrintBufferSize;
    PPI_DEVICE_EXTENSION _DevExt;     
    //////////////////////////////////////////////////////////////////////////
    // Private methods
    // 
    VOID Destroy();
    static VOID PI_SetSysenterCallback( __in_opt ULONG CpuNumber, __in_opt PVOID Context );
    NTSTATUS PI_PerformMemoryOperation( PI_OPERATION_TYPE OperationType, HANDLE ProcessId, PVOID TargetAddress, PVOID Buffer, ULONG BuffserSize );
    NTSTATUS PI_RegisterProcess( __in PPI_PROCESS_LIST List, __in HANDLE ProcessId );
    NTSTATUS PI_UnregisterProcess( __in PPI_PROCESS_LIST List, __in HANDLE ProcessId );
    NTSTATUS PI_UnregisterAllProcesses( __in PPI_PROCESS_LIST List );
    VOID PI_DropAllConnections( __in PPI_PROCESS_LIST List );
public:
    ProcessIsolator( __in PPI_DEVICE_EXTENSION DevExt );
    ~ProcessIsolator();
    BOOLEAN PI_Init();    
    NTSTATUS PI_Start();
    NTSTATUS PI_Stop();
    NTSTATUS PI_ProcessIoctlOperation( __in PPI_OPERATION_REQUEST Operation );
    PI::LogFile *PI_Log()
    {
        return _Log;
    }
    HANDLE PI_LogHandle()
    {
        return _LogHandle;
    }
    PI::ProcessList *PI_ProcessList()
    {
        return _ProcessList;
    }
    PPI_PROCESS_LIST PI_List()
    {
        return _List;
    }
    PI::Memory *PI_Memory()
    {
        return _Mem;
    }
};

NAMESPACE_PI_END

extern PI_DEVICE_EXTENSION g_PIDevExt;

#define PIDevExt (g_PIDevExt)
#define PILog ((PI::LogFile &)*( g_PIDevExt.Pi->PI_Log()))
#define PILogHandle ( g_PIDevExt.Pi->PI_LogHandle() )
#define PIMem ((PI::Memory &)*g_PIDevExt.Pi->PI_Memory())
#define PIProcessList ( (PI::ProcessList &)(*g_PIDevExt.Pi->PI_ProcessList()) )
#define PIList ( (PPI_PROCESS_LIST)(g_PIDevExt.Pi->PI_List()) )
#define PIClient( Entry )( ((PI::LpcClient *)Entry->Client ) )