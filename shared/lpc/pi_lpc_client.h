//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_lpc_client.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator LPC client header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "pi_lpc_common.h"

NAMESPACE_PI_BEGIN

class LpcClient : public LpcBase
{
#ifdef _PI_KRN
private:
    static VOID SystemThreadRoutineConnect( __in PVOID StartContext );
#endif
public:
    LpcClient( Memory *mem );
    ~LpcClient();    
    _Success_(return)
        BOOLEAN Connect( __in PWCHAR PortName );
    VOID CloseConnection()
    {
        if( NULL != _PortHandle )
        {
            ClosePort( _PortHandle );
            _PortHandle = NULL;
        }
    }
    _Success_(return)
        BOOLEAN SendMsg( __in PVOID Msg, __in ULONG MsgSize );
    _Success_(return)
        BOOLEAN SendMsgWaitReply( __in PVOID Msg, __in ULONG MsgSize, __out PVOID Reply );
    _Success_(return)
        BOOLEAN SendMsgReply( __in HANDLE PortHandle, __in PVOID Reply, __in ULONG ReplySize );
};

NAMESPACE_PI_END