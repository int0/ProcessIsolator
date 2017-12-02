#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <pi/pi.h>
#include <nt/nt_names_win7_x64.h>
#include "mem/pi_new.h"
#include "pi/pi_drv.h"
#include "pi/pi_server.h"

typedef enum NT_STATUS
{
    STATUS_ACCESS_DENIED = ((NTSTATUS)0xC0000022L),
};

CHAR LogBuffer[4096];

PI::PIServer piServer;
PI::LogFile piLog( &LogBuffer[0], sizeof(LogBuffer) );
PI::PIDrv piDrv;

VOID PI::PIServer::PISysEnterHandler( __in PI::PPI_SRV_SYSENTER_INFO Info, __inout PI::REG_CONTEXT *Context, __out PI::PPI_SYSENTER_REPLY Reply )
{
#ifdef _AMD64_
    PISysEnter64Handler( Info, Context, Reply );        
#elif _X86_
    PISysEnter32Handler( Info, Context, Reply );        
#endif
}

VOID PI::PIServer::PISysEnter32Handler( __in PI::PPI_SRV_SYSENTER_INFO Info, __inout PI::REG32_CONTEXT *Context, __out PI::PPI_SYSENTER_REPLY Reply )
{

}

//
// r10, rdx, r8, and r9 - Supply the first four system call arguments
//
VOID PI::PIServer::PISysEnter64Handler( __in PI::PPI_SRV_SYSENTER_INFO Info, __inout PI::REG64_CONTEXT *Context, __out PI::PPI_SYSENTER_REPLY Reply )
{
    PCHAR FuncName;
    ULONG SvcNum = Info->SysServiceNumber;

    Reply->Execute = TRUE;
    Reply->Status = STATUS_ACCESS_DENIED;

    if( 0 != ( SvcNum >> 12 ) )
        FuncName = WIN32K_WIN7_SP1_X64_FNAMES[SvcNum & 0xFFF];
    else
        FuncName = NT_WIN7_SP1_X64_FNAMES[SvcNum & 0xFFF];   

    piLog.Print( _LogHandle, "%-6d %-6d | %-8d %s", 
        (ULONG)Info->Cid.UniqueProcess,
        (ULONG)Info->Cid.UniqueThread, SvcNum, FuncName );
}

VOID __stdcall PIServerCallback( __in PI::PIServer *Srv, __in PI::PPI_CLIENT_ID Cid, __in PI::PPI_SYSENTER_REQUEST Request, __out PI::PPI_SYSENTER_REPLY Reply, __in_opt PVOID Context )
{    
    PI::PI_OPERATION_REQUEST DrvOpReq;
    PI::REG_CONTEXT RegContext;


    Reply->Execute = FALSE;
    Reply->Status = STATUS_ACCESS_DENIED;

    DrvOpReq.Type = PI::ReadMemory;
    DrvOpReq.Memory.Address = Request->RegContext;
    DrvOpReq.Memory.Buffer = &RegContext;
    DrvOpReq.Memory.BufferSize = sizeof(RegContext);

    if( Srv->Drv()->SendOperationRequest( &DrvOpReq ) )
    {
        PI::PI_SRV_SYSENTER_INFO Info = { Request->ServiceNumber, *Cid };
        
        Srv->PISysEnterHandler( &Info, &RegContext, Reply );
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    PCHAR ErrorMsg;    

    printf( "Starting up ProcessIsolator service ...\n" );

    //
    // Initialize global memory manager
    //
    if( !piMem.Init() )
    {
        ErrorMsg = "Failed to initialize memory manager";
        goto _ReportError;
    }
   
    if( !piDrv.Init() )
    {
        ErrorMsg = "Unable to open ProcessIsolator device driver";
        goto _ReportError;
    }

    if( !piServer.Init( &piDrv, &piMem, &piLog, PIServerCallback, NULL ) )
    {
        ErrorMsg = "Unable to initialize ProcessIsolator server";
        goto _ReportError;
    }

    if( !piServer.Start() )
    {
        ErrorMsg = "Unable to start ProcessIsolator server";
        goto _ReportError;
    }

    printf( "ProcessIsolator running ...\n" );

    Sleep( -1 );

_ReportError:
    printf( "[ERROR] %s\n", ErrorMsg );
    return 0;
}

