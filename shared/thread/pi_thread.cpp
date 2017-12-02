#include <pi/pi.h>

HANDLE PIThread::ThreadCreate( __in PI_THREAD_START_ROUTINE ThreadStart, __in_opt ULONG CreationFlags, __in_opt PVOID Params, __out PHANDLE ThreadId )
{
    UNREFERENCED_PARAMETER( CreationFlags );
#ifdef _PI_KRN
    CLIENT_ID Cid = {NULL,NULL};
    HANDLE ThHandle = NULL;
#pragma warning(suppress: 28023)
    if( NT_SUCCESS( PsCreateSystemThread( &ThHandle, THREAD_ALL_ACCESS, NULL, NULL, &Cid, (PKSTART_ROUTINE)ThreadStart, Params  ) ) )
    {
        *ThreadId = Cid.UniqueThread;
        return ThHandle;
    }
    *ThreadId = NULL;
    return NULL;
#else    
    return CreateThread( NULL, 0, (PTHREAD_START_ROUTINE)ThreadStart, (PVOID)Params, CreationFlags, (PULONG)ThreadId );
#endif
}

BOOLEAN PIThread::Close()
{
#ifdef _PI_KRN
    return NT_SUCCESS( ZwClose( _ThreadHandle ) );
#else
    return CloseHandle( _ThreadHandle );
#endif
}

BOOLEAN PIThread::Start()
{
    return ( NULL != ( _ThreadHandle = ThreadCreate( _StartRoutine, _CreationFlags, _ThreadParams, &_ThreadId ) ) );
}

/// <summary>
/// Terminates thread
/// </summary>
VOID PIThread::Stop()
{
#ifdef _PI_USR
    if( _ThreadHandle )
        TerminateThread( _ThreadHandle, 0 );
#endif
}