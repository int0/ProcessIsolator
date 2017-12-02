//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_lpc_server.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator LPC server implementation
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>
#include <thread/pi_thread.h>

#ifdef _PI_KRN
extern "C"
{

typedef NTSTATUS (NTAPI *T_NtCreatePort)(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage );

typedef NTSTATUS (NTAPI *T_NtAcceptConnectPort)(
    __out PHANDLE PortHandle,
    __in_opt PVOID PortContext,
    __in PPORT_MESSAGE ConnectionRequest,
    __in BOOLEAN AcceptConnection,
    __inout_opt PPORT_VIEW ServerView,
    __out_opt PREMOTE_PORT_VIEW ClientView );

typedef NTSTATUS (NTAPI *T_NtCompleteConnectPort)( 
    __in HANDLE PortHandle );

typedef NTSTATUS (NTAPI *T_NtReplyWaitReceivePort)(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext ,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage );

    T_NtCreatePort NtCreatePort = NULL;
    T_NtCompleteConnectPort NtCompleteConnectPort = NULL;
    T_NtAcceptConnectPort NtAcceptConnectPort = NULL;
    T_NtReplyWaitReceivePort NtReplyWaitReceivePort = NULL;
};
#endif

NAMESPACE_PI_BEGIN
#pragma warning(suppress: 28253)
LpcServer::LpcServer( __in Memory *mem, __in LPC_SRV_CALLBACK LpcMsgCallback, __in_opt PVOID Context )
{
    _Mem = mem;
    _PortHandle = NULL;
    _MaxMsgLen = PORT_MAXIMUM_MESSAGE_LENGTH;
    _LpcCallback = LpcMsgCallback;
    _Context = Context;
    _ComHandleList = NULL;
}

LpcServer::~LpcServer()
{
    ClosePort( _PortHandle );
}

BOOLEAN LpcServer::Create( __in_z PWCHAR ServerName )
{
    NTSTATUS Status;
    ULONG Attributes;
    UNICODE_STRING SrvPortName_UStr;
    OBJECT_ATTRIBUTES ObjAttr;
    
    RtlInitUnicodeString( &SrvPortName_UStr, ServerName );

#ifdef _PI_KRN
    Attributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
#else
    Attributes = OBJ_CASE_INSENSITIVE;
#endif

    InitializeObjectAttributes( &ObjAttr, &SrvPortName_UStr, Attributes, NULL, NULL );
    
    Status = NtCreatePort( &_PortHandle, &ObjAttr, 0, _MaxMsgLen, 0 );

    if( NT_SUCCESS(Status) )
    {
        PSRV_LISTENER_PARAMS Params = 
            (PSRV_LISTENER_PARAMS)this->Mem()->Alloc( sizeof(*Params) );
        
        if( NULL != Params )
        {
            Params->Server = this;
            Params->Callback = _LpcCallback;
            Params->PortHandle = _PortHandle;
            Params->Ctx = _Context;
            
            PIThread pit( &LpcServer::ServerListener, 0, Params );

            if( pit.Start() )
            {
                return TRUE;
            }
            else
            {
                this->Mem()->Free( Params );
            }
        }
        ClosePort( _PortHandle );
        _PortHandle = NULL;
    }
    else
    {

    }

    return FALSE;
}

ULONG __stdcall LpcServer::ServerListener( __in PVOID ThreadParam )
{
    NTSTATUS Status;
    PLPC_MESSAGE Message;
    SRV_LISTENER_PARAMS Params;
    UCHAR PortMsgBuff[sizeof(*Message)];
    Message = (PLPC_MESSAGE)&PortMsgBuff;

    Params.Server = ((PSRV_LISTENER_PARAMS)ThreadParam)->Server;
    Params.PortHandle = ((PSRV_LISTENER_PARAMS)ThreadParam)->PortHandle;
    Params.Callback = ((PSRV_LISTENER_PARAMS)ThreadParam)->Callback;
    Params.Ctx = ((PSRV_LISTENER_PARAMS)ThreadParam)->Ctx;

#ifdef _PI_USR
    printf( __FUNCTION__" TID: %d Entered\n", GetCurrentThreadId() );
#endif
    
    memset( PortMsgBuff, 0, sizeof(PortMsgBuff) );
    Params.Server->Mem()->Free( ThreadParam );

    do 
    {
        Status = NtReplyWaitReceivePort( Params.PortHandle, NULL, NULL, &Message->Hdr );

        if( NT_SUCCESS(Status) )
        {
            switch( Message->Hdr.u2.s2.Type )
            {
            case LPC_CONNECTION_REQUEST:
                {
                    HANDLE ComHandle;

                    Status = NtAcceptConnectPort( &ComHandle, NULL, &Message->Hdr, TRUE, NULL, NULL );

                    if( NT_SUCCESS( Status ) )
                    {
                        if( !NT_SUCCESS( Status = NtCompleteConnectPort( ComHandle ) ) )
                        {
                            Params.Server->ClosePort( ComHandle );
                        }
                        else
                        {
                            Params.Server->AddComHandle( ComHandle );
                        }                        
                    }
                }break;
//             case LPC_LOST_REPLY:
//                 {
// 
//                 }break;
            case LPC_CLIENT_DIED: 
            case LPC_PORT_CLOSED:
                 {
                     if( Params.PortHandle != Params.Server->ServerConnectionPort() )
                     {
                         Params.Server->ClosePort( Params.PortHandle );
                     }
                 }break;
            default:
                Params.Callback( Params.Server, Params.PortHandle, Message, Params.Ctx );
                break;
            }  
        }

    } while( NT_SUCCESS(Status) );

#ifdef _PI_USR
    printf( __FUNCTION__" TID: %d Exit\n", GetCurrentThreadId() );
#endif // _PI_USR

    return 0;
}

NAMESPACE_PI_END