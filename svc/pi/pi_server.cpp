#include <pi/pi.h>
#include "pi_drv.h"
#include "pi_server.h"

NAMESPACE_PI_BEGIN
PIServer::PIServer()
{
    _Lpc = NULL;    
    _Log = NULL;
    _Mem = NULL;
    _Drv = NULL;
    _LogHandle = NULL;
}
PIServer::~PIServer()
{
    Destroy();
}
VOID PIServer::RunCallback( __in PI::PPI_CLIENT_ID Cid, __in PI::PPI_SYSENTER_REQUEST Request, __in PI::PPI_SYSENTER_REPLY Reply )
{
    return _Callback( this, Cid, Request, Reply, _Context );
}
BOOLEAN PIServer::Init( __in PI::PIDrv *drv, __in PI::Memory *mem, __in PI::LogFile *log, __in PI::PI_SERVER_CALLBACK Callback, __in_opt PVOID Context )
{
    BOOLEAN Res = FALSE;
    _PIFolderMaxSize = MAX_PATH;    
    _Drv = drv;
    _Mem = mem;
    _Log = log;
    _Context = Context;
    _Callback = Callback;

    if( NULL == ( _PIFolder = (PWCHAR)mem->Alloc( _PIFolderMaxSize * sizeof(WCHAR) ) ) )
        goto _Fail;
    
    if( _PIFolderMaxSize < GetEnvironmentVariableW( L"SystemDrive", _PIFolder, _PIFolderMaxSize ) )
        goto _Fail;

    if( !SUCCEEDED( StringCchCatW( _PIFolder, _PIFolderMaxSize, L"\\PISO" ) ) )
        goto _Fail;

    if( INVALID_FILE_ATTRIBUTES == GetFileAttributesW( _PIFolder ) )
        CreateDirectoryW( _PIFolder, NULL );

    _LogHandle = log->Create( _PIFolder, L"pi_srv_", GetCurrentProcessId() );

    if( NULL == _LogHandle )
        goto _Fail;

    _Lpc = new LpcServer( _Mem, &PIServer::LpcSrvCallback, this );

    if( NULL == _Lpc )
        goto _Fail;

    if( !_Lpc->Create( L"\\ProcessIsolator" ) )
        goto _Fail;

    if( NULL == ( _FolderMonThread = new PIThread( &PIServer::FolderMonThread, 0, (PVOID)this ) ) )
        goto _Fail;

    Res = TRUE;

_Fail:

    return Res;
}

ULONG __stdcall PIServer::FolderMonThread( __in_opt PVOID ThreadParams )
{
    HANDLE FindHandle;
    WIN32_FIND_DATAW Find;
    WCHAR TargetFile[MAX_PATH];
    WCHAR IncomingFolder[MAX_PATH];
    WCHAR CopyTo[MAX_PATH];
    PIServer *Srv = (PI::PIServer *)ThreadParams;
    PWCHAR PIFolder = Srv->GetWorkingFolder();

    StringCchPrintfW( IncomingFolder, _countof(IncomingFolder), L"%s\\in", PIFolder );

    if( INVALID_FILE_ATTRIBUTES == GetFileAttributesW(IncomingFolder) )
        CreateDirectoryW( IncomingFolder, NULL );

    StringCchCatW( IncomingFolder, _countof(IncomingFolder), L"\\*" );

    for (;;)
    {        
        if( INVALID_HANDLE_VALUE !=( FindHandle = FindFirstFileW( IncomingFolder, &Find ) ) )
        {
            do 
            {
                if( FILE_ATTRIBUTE_DIRECTORY == 
                    ( Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
                    continue;

                StringCchPrintfW( TargetFile, _countof(TargetFile), L"%s\\in\\%s", PIFolder, Find.cFileName );
                StringCchPrintfW( CopyTo, _countof(CopyTo), L"%c:\\%s", PIFolder[0], Find.cFileName );
                CopyFileW( TargetFile, CopyTo, FALSE );
                DeleteFileW( TargetFile );
                Srv->StartIsolatedProcess( CopyTo, NULL );
                DeleteFileW( CopyTo );
            } while ( FindNextFileW( FindHandle, &Find ) );

            FindClose( FindHandle );
            Sleep(100);
        }
        else
        {
            Sleep( 5000 );  ///< If there was error sleep for 5 sec
        }       
    }
}

VOID PIServer::Destroy()
{
    if( NULL != _LogHandle )
        _Log->Close( _LogHandle );

    if( NULL != _Lpc )
        delete _Lpc;

    if( NULL != _PIFolder )
    {
        _Mem->Free( _PIFolder );
        _PIFolder = NULL;
    }

    if( NULL != _FolderMonThread )
    {
        _FolderMonThread->Stop();
        delete _FolderMonThread;
    }
}

BOOLEAN PIServer::Start()
{
    PI::PI_OPERATION_REQUEST Operation;

    Operation.Type = StartProcessIsolator;

    //
    // Tell driver to start monitoring
    // 
    if( !_Drv->SendOperationRequest( &Operation ) )
        return FALSE;

    //
    // Start monitoring incoming folder for files for replication
    //
    if( !_FolderMonThread->Start() )
        return FALSE;
    
    return TRUE;
}

BOOLEAN PIServer::StartIsolatedProcess( __in_opt PWCHAR FilePath, __in_opt PWCHAR CommandLine )
{
    BOOLEAN Res = FALSE;
    STARTUPINFOW sinfo = { sizeof(sinfo) };
    PROCESS_INFORMATION pinfo;
    PI::PI_OPERATION_REQUEST PIRequest;   
    
    GetStartupInfoW( &sinfo );

    if( CreateProcessW( FilePath, CommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &sinfo, &pinfo ) )
    {
        PIRequest.Type =  PI::RegisterProcess;
        PIRequest.ProcessId = (HANDLE)pinfo.dwProcessId;

        if( _Drv->SendOperationRequest( &PIRequest ) )
        {
            Sleep( 5 );  /// Wait client-server connection
            ResumeThread( pinfo.hThread );
            Res = TRUE;
        }
        else
        {
            ::TerminateProcess( pinfo.hProcess, 0 );
            WaitForSingleObject( pinfo.hProcess, 1000 );
        }        
        
        CloseHandle( pinfo.hProcess );
        CloseHandle( pinfo.hThread );
    }

    return Res;
}

VOID __stdcall PIServer::LpcSrvCallback( __in PI::LpcServer *Server, __in HANDLE PortHandle, __in PI::PLPC_MESSAGE PortMsg, __in_opt PVOID Context )
{
    PI::PIServer *piSrv = (PI::PIServer *)Context;
    LPC_MSG_TYPE Type = GetPortMessageType( PortMsg );
    ULONG PortDataLen = GetPortMessageDataLen( PortMsg );
    ULONG Pid = (ULONG)GetPortMessagePid(PortMsg);
    ULONG Tid = (ULONG)GetPortMessageTid(PortMsg);

    if( LPC_REQUEST != Type )
        printf( "MsgType: %s (%d)\n", LPC_MSG_TYPE_NAME[Type], Type );

    switch ( Type )
    {
    case LPC_DATAGRAM:              ///< Client doesn't wait for reply
        {

        }break;
    case LPC_REQUEST:               ///< Client awaits for reply
        {
            PI::PI_SYSENTER_REPLY Reply;
            PI::PPI_SYSENTER_REQUEST Request = (PI::PPI_SYSENTER_REQUEST)GetPortMessageData( PortMsg );

            if( PortDataLen == sizeof(*Request) )
            {
                PI::PI_CLIENT_ID Cid = { PortMsg->Hdr.ClientId.UniqueProcess, PortMsg->Hdr.ClientId.UniqueThread };
                piSrv->RunCallback( &Cid, Request, &Reply );
                Server->SendMsgReply( PortHandle, &PortMsg->Hdr, (PVOID)&Reply, sizeof(Reply) );
            }
            else
            {
                printf( "ERROR: expected MsgLen = %d\n", sizeof(*Request) );
            } 
        }break;
    default:
        break;
    }
}

NAMESPACE_PI_END

