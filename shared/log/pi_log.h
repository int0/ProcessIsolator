//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_log.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator logging header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once

NAMESPACE_PI_BEGIN
class LogFile
{
private:
    PCHAR _PrintBuffer;
    ULONG _PrintBufferSize;
#ifdef _PI_KRN
    KMUTEX _KMutex;
#else
    CRITICAL_SECTION _CritSec;
#endif
    VOID Unlock();
    BOOLEAN Lock();
    VOID LockInit();
    VOID AddLine( __in HANDLE LogHandle, __in PCHAR Msg, __in ULONG MsgSizeInBytes );
public:
    LogFile( __in_ecount(LogBufferSize) PCHAR LogBuffer, __in ULONG LogBufferSize );
    HANDLE Create( __in_z PWCHAR LogFolderPath, __in_z PWCHAR LogFileName, __in_opt ULONG ProcessId );
    VOID Print( __in HANDLE LogHandle, __in_z __format_string PCHAR format, ... );
    VOID Close( __in HANDLE LogHandle );
};

#define LOG_SRC_INFO __FUNCTION__"() "__FILE__":", __LINE__    ///< "FunctionName() source.cpp:42"

NAMESPACE_PI_END
