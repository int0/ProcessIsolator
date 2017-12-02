//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_thread.h
/// Author:         Volodymyr Pikhur
/// Description:    Threads related functions header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once

typedef ULONG (__stdcall *PI_THREAD_START_ROUTINE)( __in_opt PVOID ThreadParams );

class PIThread
{
private:
    HANDLE _ThreadId;
    HANDLE _ThreadHandle;
    PI_THREAD_START_ROUTINE _StartRoutine;
    ULONG _CreationFlags;
    PVOID _ThreadParams;
    BOOLEAN Close();
    HANDLE ThreadCreate( __in PI_THREAD_START_ROUTINE ThreadStart, __in_opt ULONG CreationFlags, __in_opt PVOID Params, __out PHANDLE ThreadId );
public:
    PIThread( __in PI_THREAD_START_ROUTINE StartRoutine, __in_opt ULONG CreationFlags, __in_opt PVOID ThreadParams )
    {
        _ThreadId = 0;
        _ThreadHandle = NULL;
        _StartRoutine = StartRoutine;
        _CreationFlags = CreationFlags;
        _ThreadParams = ThreadParams;
    }
    ~PIThread()
    {
        Close();
    }
    BOOLEAN Start();
    VOID Stop();
};