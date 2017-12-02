//////////////////////////////////////////////////////////////////////////
/// 
/// File:           nt_lpc.h
/// Author:         Volodymyr Pikhur
/// Description:    Windows LPC structures and types
/// 
//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////
// Lpc structures
// 
typedef struct _LPC_CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} LPC_CLIENT_ID, *PLPC_CLIENT_ID;

typedef struct _PORT_VIEW 
{
    ULONG Length;
    HANDLE SectionHandle;
    ULONG SectionOffset;
    SIZE_T ViewSize;
    PVOID ViewBase;
    PVOID ViewRemoteBase;
} PORT_VIEW, *PPORT_VIEW;

typedef struct _REMOTE_PORT_VIEW 
{
    ULONG Length;
    SIZE_T ViewSize;
    PVOID ViewBase;
} REMOTE_PORT_VIEW, *PREMOTE_PORT_VIEW;

//
// Port message structure data goes after this structure
//
typedef struct _PORT_MESSAGE
{
    union
    {
        struct 
        {
            SHORT DataLength;
            SHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union
    {
        struct
        {
            SHORT Type;
            SHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union 
    {
        LPC_CLIENT_ID ClientId;
        DOUBLE DoNotUseThisField;
    };
    ULONG MessageId;
    union
    {
        SIZE_T ClientViewSize;          // Used by LPC_CONNECTION_REQUEST message
        ULONG CallbackId;               // Used by LPC_REQUEST message
    };
} PORT_MESSAGE, *PPORT_MESSAGE;


typedef struct _LPC_CLIENT_DIED_MSG 
{
    PORT_MESSAGE PortMsg;
    LARGE_INTEGER CreateTime;
} LPC_CLIENT_DIED_MSG, *PLPC_CLIENT_DIED_MSG;

typedef enum _LPC_MSG_TYPE
{
    LPC_REQUEST            =  1,
    LPC_REPLY              =  2,
    LPC_DATAGRAM           =  3,
    LPC_LOST_REPLY         =  4,
    LPC_PORT_CLOSED        =  5,
    LPC_CLIENT_DIED        =  6,
    LPC_EXCEPTION          =  7,
    LPC_DEBUG_EVENT        =  8,
    LPC_ERROR_EVENT        =  9,
    LPC_CONNECTION_REQUEST = 10
} LPC_MSG_TYPE, *PLPC_MSG_TYPE;

#if defined(_AMD64_)
#define PORT_MAXIMUM_MESSAGE_LENGTH 512
#else
#define PORT_MAXIMUM_MESSAGE_LENGTH 256
#endif

#define LPC_MAX_CONNECTION_INFO_SIZE (16 * sizeof(ULONG_PTR))
#define PORT_TOTAL_MAXIMUM_MESSAGE_LENGTH ((PORT_MAXIMUM_MESSAGE_LENGTH + sizeof(PORT_MESSAGE) + LPC_MAX_CONNECTION_INFO_SIZE + 15) & ~15)


//////////////////////////////////////////////////////////////////////////
// Lpc functions
//

extern "C" NTSTATUS	NTAPI NtReadRequestData(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message,
    __in ULONG DataEntryIndex,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead );

extern "C" NTSTATUS	NTAPI NtWriteRequestData(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message,
    __in ULONG DataEntryIndex,
    __in_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten );


//
// Client APIs
// 

//
// Used by client to connect to a connection port
//
extern "C" NTSTATUS	NTAPI ZwConnectPort(
    __out PHANDLE PortHandle,
    __in PUNICODE_STRING PortName,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    __inout_opt PPORT_VIEW ClientView,
    __inout_opt PREMOTE_PORT_VIEW ServerView,
    __out_opt PULONG MaxMessageLength,
    __inout_opt PVOID ConnectionInformation,
    __inout_opt PULONG ConnectionInformationLength );

extern "C" NTSTATUS	NTAPI NtConnectPort(
    __out PHANDLE PortHandle,
    __in PUNICODE_STRING PortName,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    __inout_opt PPORT_VIEW ClientView,
    __inout_opt PREMOTE_PORT_VIEW ServerView,
    __out_opt PULONG MaxMessageLength,
    __inout_opt PVOID ConnectionInformation,
    __inout_opt PULONG ConnectionInformationLength );

//
// Used to send a datagram message that does not have a reply
//
extern "C" NTSTATUS	NTAPI NtRequestPort( 
    __in HANDLE PortHandle, 
    __in PPORT_MESSAGE RequestMessage );
//
// Used to send a message and wait for a reply
// 
extern "C" NTSTATUS	NTAPI ZwRequestWaitReplyPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE RequestMessage,
    __out PPORT_MESSAGE ReplyMessage );

//
// Server API
//////////////////////////////////////////////////////////////////////////

//
// Used by server to create a connection port
//
#ifndef _PI_KRN
extern "C" NTSTATUS	NTAPI NtCreatePort(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage
    );

extern "C" NTSTATUS	NTAPI NtCreateWaitablePort(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage
    );

//
// Used by server to listen for connection requests on the connection port
//
extern "C" NTSTATUS	NTAPI NtListenPort(
    __in HANDLE PortHandle,
    __out PPORT_MESSAGE ConnectionRequest );
//
// Used by server to accept connection requests on the connection port
//
extern "C" NTSTATUS	NTAPI NtAcceptConnectPort(
    __out PHANDLE PortHandle,
    __in_opt PVOID PortContext,
    __in PPORT_MESSAGE ConnectionRequest,
    __in BOOLEAN AcceptConnection,
    __inout_opt PPORT_VIEW ServerView,
    __out_opt PREMOTE_PORT_VIEW ClientView );
//
// Used by server to complete the acceptance of a connection request
// 
extern "C" NTSTATUS NTAPI NtCompleteConnectPort( 
    __in HANDLE PortHandle );

//
// Used by server to send a reply to the client and wait to receive a message from the client
//
extern "C" NTSTATUS	NTAPI NtReplyWaitReceivePort(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext ,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage );

extern "C" NTSTATUS	NTAPI NtReplyWaitReceivePortEx(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage,
    __in_opt PLARGE_INTEGER Timeout	);
//
// Used by server thread to temporarily borrow the security context of a client thread
// 
extern "C" NTSTATUS NTAPI NtImpersonateClientOfPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message );
#endif
//
// Used to send a reply to a particular message
// 
extern "C" NTSTATUS NTAPI NtReplyPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE ReplyMessage );
//
// Used to send a reply to a particular message and wait for a reply to a previous message
//
extern "C" NTSTATUS	NTAPI NtReplyWaitReplyPort(
    __in HANDLE PortHandle,
    __inout PPORT_MESSAGE ReplyMessage );

