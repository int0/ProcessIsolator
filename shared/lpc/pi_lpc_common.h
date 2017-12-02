//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_lpc_common.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator LPC common client/server header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once

NAMESPACE_PI_BEGIN

class LpcBase
{
private:    
    BOOLEAN SendMsgReplyInternal( __in HANDLE PortHandle, __in PPORT_MESSAGE RequestPortMsg, __in PVOID Reply, __in ULONG ReplySize );
protected:
    Memory *_Mem;
    ULONG _MaxMsgLen;
    HANDLE _PortHandle;
public:
    Memory *Mem()
    {
        return _Mem;
    }
    VOID ClosePort( __in HANDLE PortHandle );
    BOOLEAN SendMsgReply( __in HANDLE PortHandle, __in PPORT_MESSAGE RequestPortMsg, __in PVOID Reply, __in ULONG ReplySize );
    //BOOLEAN SendMsgReplyWaitReply( __in HANDLE PortHandle, __in PPORT_MESSAGE RequestPortMsg, __in PVOID Reply, __in ULONG ReplySize );
};


typedef struct _LPC_MESSAGE 
{
    PORT_MESSAGE Hdr;
    UCHAR Data[PORT_MAXIMUM_MESSAGE_LENGTH-sizeof(PORT_MESSAGE)];
} LPC_MESSAGE, *PLPC_MESSAGE;

class LpcServer;

typedef VOID (__stdcall *LPC_SRV_CALLBACK)( __in PI::LpcServer *Server, __in HANDLE PortHandle, __in PI::PLPC_MESSAGE PortMsg, __in_opt PVOID Ctx );

#define GetPortMessageType( PortMsg )       ( (LPC_MSG_TYPE)((PI::PLPC_MESSAGE)PortMsg)->Hdr.u2.s2.Type )
#define GetPortMessageDataLen( PortMsg )    ( ((PI::PLPC_MESSAGE)PortMsg)->Hdr.u1.s1.DataLength )
#define GetPortMessageData( PortMsg )       ( (PVOID)(&((PI::PLPC_MESSAGE)PortMsg)->Data[0]) )
#define GetPortMessagePid( PortMsg )        ( ((PI::PLPC_MESSAGE)PortMsg)->Hdr.ClientId.UniqueProcess )
#define GetPortMessageTid( PortMsg )        ( ((PI::PLPC_MESSAGE)PortMsg)->Hdr.ClientId.UniqueThread )

extern PCHAR LPC_MSG_TYPE_NAME[];

NAMESPACE_PI_END