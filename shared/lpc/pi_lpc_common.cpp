//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_lpc_common.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator LPC common client/server implementation
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>

NAMESPACE_PI_BEGIN

PCHAR LPC_MSG_TYPE_NAME[] =
{
    "LPC_INVALID_ZERO_TYPE",
    "LPC_REQUEST",              //   LPC_REQUEST            =  1,
    "LPC_REPLY",                //   LPC_REPLY              =  2,
    "LPC_DATAGRAM",             //   LPC_DATAGRAM           =  3,
    "LPC_LOST_REPLY",           //   LPC_LOST_REPLY         =  4,
    "LPC_PORT_CLOSED",          //   LPC_PORT_CLOSED        =  5,
    "LPC_CLIENT_DIED",          //   LPC_CLIENT_DIED        =  6,
    "LPC_EXCEPTION",            //   LPC_EXCEPTION          =  7,
    "LPC_DEBUG_EVENT",          //   LPC_DEBUG_EVENT        =  8,
    "LPC_ERROR_EVENT",          //   LPC_ERROR_EVENT        =  9,
    "LPC_CONNECTION_REQUEST"    //   LPC_CONNECTION_REQUEST = 10
};

VOID LpcBase::ClosePort( __in HANDLE PortHandle )
{
    if( PortHandle != NULL )
    {
#ifdef _PI_KRN
        ZwClose( PortHandle );
#else
        NtClose( PortHandle );
#endif
    }
}

#ifndef _PI_KRN
BOOLEAN LpcBase::SendMsgReplyInternal( __in HANDLE PortHandle, __in PPORT_MESSAGE RequestPortMsg, __in PVOID Reply, __in ULONG ReplySize )
{
    NTSTATUS Status;
    LPC_MESSAGE ReplyMsg;

    if( ReplySize > sizeof(ReplyMsg.Data) )
        return FALSE;

    memset( &ReplyMsg.Hdr, 0, sizeof(ReplyMsg.Hdr) );
    memcpy( &ReplyMsg.Data[0], Reply, ReplySize );

    ReplyMsg.Hdr.u1.s1.TotalLength = (USHORT)_MaxMsgLen;
    ReplyMsg.Hdr.u1.s1.DataLength = (USHORT)ReplySize;
    ReplyMsg.Hdr.MessageId = RequestPortMsg->MessageId;
    ReplyMsg.Hdr.ClientId = RequestPortMsg->ClientId;    

    Status = NtReplyPort( PortHandle, &ReplyMsg.Hdr );        

    if( NT_SUCCESS( Status ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOLEAN LpcBase::SendMsgReply( __in HANDLE PortHandle, __in PPORT_MESSAGE RequestPortMsg, __in PVOID Reply, __in ULONG ReplySize )
{
    return SendMsgReplyInternal( PortHandle, RequestPortMsg, Reply, ReplySize );
}

// BOOLEAN CLpcBase::SendMsgReplyWaitReply( __in HANDLE PortHandle, __in PPORT_MESSAGE RequestPortMsg, __in PVOID Reply, __in ULONG ReplySize )
//{
//    return SendMsgReplyInternal( PortHandle, RequestPortMsg, Reply, ReplySize, TRUE );
//}
//} 
#endif

NAMESPACE_PI_END