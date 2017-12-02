//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_lpc_client.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator LPC client implementation
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>

NAMESPACE_PI_BEGIN

#ifdef _PI_KRN
    typedef struct _SYSTEM_PORT_CONNECT_CTX
    {
        KEVENT Event;
        NTSTATUS Status;        
        PULONG MaxMsgLen;
        PHANDLE PortHandle;        
        PUNICODE_STRING PortName_UStr;
        PSECURITY_QUALITY_OF_SERVICE Sqos;
    } SYSTEM_PORT_CONNECT_CTX, *PSYSTEM_PORT_CONNECT_CTX;

    VOID LpcClient::SystemThreadRoutineConnect( __in PVOID StartContext )
    {
        PSYSTEM_PORT_CONNECT_CTX Ctx = (PSYSTEM_PORT_CONNECT_CTX)StartContext;

        Ctx->Status = ZwConnectPort( Ctx->PortHandle, Ctx->PortName_UStr, Ctx->Sqos, NULL, NULL, Ctx->MaxMsgLen, NULL, NULL );

        KeSetEvent( &Ctx->Event, 0, FALSE );

        PsTerminateSystemThread( STATUS_SUCCESS );
    }
#endif
    LpcClient::LpcClient( Memory *mem )
    {
        _Mem = mem;
        _PortHandle = NULL;
        _MaxMsgLen = 0;
    }

    LpcClient::~LpcClient()
    {
        ClosePort( _PortHandle );
    }

_Success_(return)
BOOLEAN LpcClient::Connect( __in PWCHAR PortName )
{
    NTSTATUS Status;
    BOOLEAN Result = FALSE;    
    UNICODE_STRING PortName_UStr;
    SECURITY_QUALITY_OF_SERVICE Sqos;

    RtlInitUnicodeString( &PortName_UStr, PortName );

    Sqos.Length = sizeof(Sqos);
    Sqos.ImpersonationLevel = SecurityImpersonation;
    Sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    Sqos.EffectiveOnly = TRUE;

    //
    // We need to execute ZwConnectPort in System context
    //
#ifdef _PI_KRN
    CLIENT_ID Cid;
    HANDLE ThreadHandle;
    SYSTEM_PORT_CONNECT_CTX Ctx;
    
    Ctx.Sqos = &Sqos;
    Ctx.PortName_UStr = &PortName_UStr;
    Ctx.MaxMsgLen = &_MaxMsgLen;
    Ctx.PortHandle = &_PortHandle;

    KeInitializeEvent( &Ctx.Event, NotificationEvent, FALSE );

#pragma warning(suppress: 28023)
    if( NT_SUCCESS( Status = PsCreateSystemThread( &ThreadHandle, THREAD_ALL_ACCESS, NULL, NULL, &Cid, &LpcClient::SystemThreadRoutineConnect, (PVOID)&Ctx ) ) )
    {
        if( NT_SUCCESS( Status = KeWaitForSingleObject( &Ctx.Event, Executive, KernelMode, FALSE, NULL ) ) )
        {
            Result = NT_SUCCESS( Ctx.Status );
        }
    }
#else
    if( NT_SUCCESS( Status = NtConnectPort( &_PortHandle, &PortName_UStr, &Sqos, NULL, NULL, &_MaxMsgLen, NULL, NULL ) ) )
    {
        Result = TRUE;
    }
#endif

    return Result;
}

_Success_(return)
BOOLEAN LpcClient::SendMsg( __in PVOID Msg, __in ULONG MsgSize )
{
    NTSTATUS Status;
    LPC_MESSAGE RequestMsg;

    if( MsgSize > sizeof(RequestMsg.Data) )
        return FALSE;
    
    memset( &RequestMsg.Hdr, 0, sizeof(RequestMsg.Hdr) );
    memcpy( &RequestMsg.Data[0], Msg, MsgSize );

    RequestMsg.Hdr.u1.s1.TotalLength = (USHORT)_MaxMsgLen;
    RequestMsg.Hdr.u1.s1.DataLength = (USHORT)MsgSize;

    return NT_SUCCESS(Status = NtRequestPort( _PortHandle, &RequestMsg.Hdr ));
}

_Success_(return)
BOOLEAN LpcClient::SendMsgWaitReply( __in PVOID Msg, __in ULONG MsgSize, __out PVOID Reply )
{
    NTSTATUS Status;
    BOOLEAN Ret = FALSE;
    PLPC_MESSAGE RequestMsg;
    PLPC_MESSAGE ReplyMsg;

    if( MsgSize > sizeof(RequestMsg->Data) )
        return Ret;

    RequestMsg = (PLPC_MESSAGE)_Mem->Alloc( sizeof(*RequestMsg) );
    ReplyMsg = (PLPC_MESSAGE)_Mem->Alloc( sizeof(*ReplyMsg) );

    if( NULL == ReplyMsg || 
        NULL == RequestMsg )
        goto _Cleanup;

    memset( &RequestMsg->Hdr, 0, sizeof(RequestMsg->Hdr) );
    memset( &ReplyMsg->Hdr, 0, sizeof(ReplyMsg->Hdr) );
    memcpy( &RequestMsg->Data[0], Msg, MsgSize );

    RequestMsg->Hdr.u1.s1.TotalLength = (USHORT)_MaxMsgLen;
    RequestMsg->Hdr.u1.s1.DataLength = (USHORT)MsgSize;

    ReplyMsg->Hdr.u1.s1.TotalLength = (USHORT)_MaxMsgLen;
    ReplyMsg->Hdr.u1.s1.DataLength = (USHORT)sizeof(ReplyMsg->Data);

    if( NT_SUCCESS( Status = ZwRequestWaitReplyPort( _PortHandle, &RequestMsg->Hdr, &ReplyMsg->Hdr )) )
    {
        memcpy( Reply, ReplyMsg->Data, ReplyMsg->Hdr.u1.s1.DataLength );
        Ret = TRUE;
    }

_Cleanup:

    _Mem->Free( ReplyMsg );
    _Mem->Free( RequestMsg );

    return Ret;
}


NAMESPACE_PI_END