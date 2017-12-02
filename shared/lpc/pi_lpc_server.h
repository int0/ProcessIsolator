//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_lpc_server.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator LPC server header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "pi_lpc_common.h"

NAMESPACE_PI_BEGIN

typedef struct _PI_LPC_HANDLE_LIST 
{
    struct _PI_LPC_HANDLE_LIST *Next;    
    HANDLE Handle;
} PI_LPC_HANDLE_LIST, *PPI_LPC_HANDLE_LIST;

class LpcServer : public LpcBase
{
private:
    Memory *_Mem;
    PVOID _Context;
    PPI_LPC_HANDLE_LIST _ComHandleList;
    LPC_SRV_CALLBACK _LpcCallback;
    static ULONG __stdcall ServerListener( __in PVOID ThreadParam );    
public:
    LpcServer( __in Memory *mem, __in LPC_SRV_CALLBACK LpcSrvCallback, __in_opt PVOID Ctx );
    ~LpcServer();
    BOOLEAN Create( __in_z PWCHAR ServerName );
    HANDLE ServerConnectionPort()
    {
        return _PortHandle;
    }
    Memory *Mem()
    {
        return _Mem;
    }
    VOID AddComHandle( HANDLE Handle )
    {
        PPI_LPC_HANDLE_LIST Entry = 
            (PPI_LPC_HANDLE_LIST)_Mem->Alloc( sizeof(PI_LPC_HANDLE_LIST) );

        if( NULL == Entry )
            return;
        
        Entry->Handle = Handle;
        Entry->Next = _ComHandleList;
        _ComHandleList = Entry;
    }
};

typedef struct _SRV_LISTENER_PARAMS
{
    LpcServer *Server;
    HANDLE PortHandle;
    LPC_SRV_CALLBACK Callback;
    PVOID Ctx;
} SRV_LISTENER_PARAMS, *PSRV_LISTENER_PARAMS;

NAMESPACE_PI_END