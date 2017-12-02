//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_server.h
/// Author:         Volodymyr Pikhur
/// Description:    
///
//////////////////////////////////////////////////////////////////////////
#pragma once

NAMESPACE_PI_BEGIN

class PIServer;

typedef struct _PI_SRV_SYSENTER_INFO
{
    ULONG SysServiceNumber;
    PI::PI_CLIENT_ID Cid;

} PI_SRV_SYSENTER_INFO, *PPI_SRV_SYSENTER_INFO;

typedef VOID ( __stdcall *PI_SERVER_CALLBACK)( __in PI::PIServer *Srv, __in PI::PPI_CLIENT_ID Cid, __in PI::PPI_SYSENTER_REQUEST Request, __out PI::PPI_SYSENTER_REPLY Reply, __in_opt PVOID Context );

class PIServer
{
private:
    LpcServer *_Lpc;
    LogFile *_Log;
    Memory *_Mem;
    PIDrv *_Drv;
    HANDLE _LogHandle;
    PWCHAR _PIFolder;
    ULONG _PIFolderMaxSize;
    PVOID _Context;
    PIThread *_FolderMonThread;
    PI_SERVER_CALLBACK _Callback;
    VOID Destroy();    
    static ULONG __stdcall FolderMonThread( __in_opt PVOID ThreadParams );
    static VOID __stdcall LpcSrvCallback( __in PI::LpcServer *Server, __in HANDLE PortHandle, __in PI::PLPC_MESSAGE PortMsg, __in_opt PVOID Context );
    VOID PISysEnter32Handler( __in PI::PPI_SRV_SYSENTER_INFO Info, __inout PI::REG32_CONTEXT *Context, __out PI::PPI_SYSENTER_REPLY Reply );
    VOID PISysEnter64Handler( __in PI::PPI_SRV_SYSENTER_INFO Info, __inout PI::REG64_CONTEXT *Context, __out PI::PPI_SYSENTER_REPLY Reply );
public:
    PIServer();
    ~PIServer();
    PIDrv *Drv()
    {
        return _Drv;
    }
    HANDLE LogHandle()
    {
        return _LogHandle;
    }
    PWCHAR GetWorkingFolder()
    {
        return _PIFolder;
    }
    VOID RunCallback( __in PI::PPI_CLIENT_ID Cid, __in PI::PPI_SYSENTER_REQUEST Request, __in PI::PPI_SYSENTER_REPLY Reply );
    BOOLEAN Init( __in PI::PIDrv *drv, __in PI::Memory *mem, __in PI::LogFile *log, __in PI_SERVER_CALLBACK Callback, __in_opt PVOID Context );
    BOOLEAN Start();
    BOOLEAN StartIsolatedProcess( __in_opt PWCHAR FilePath, __in_opt PWCHAR CommandLine );
    VOID PISysEnterHandler( __in PI::PPI_SRV_SYSENTER_INFO Info, __inout PI::REG_CONTEXT *Context, __out PI::PPI_SYSENTER_REPLY Reply );
};

NAMESPACE_PI_END