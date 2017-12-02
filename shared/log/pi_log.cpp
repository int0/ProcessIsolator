//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_log.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator logging implementation header file
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>
#include <stdlib.h>

NAMESPACE_PI_BEGIN

    LogFile::LogFile( __in_ecount(LogBufferSize) PCHAR LogBuffer, __in ULONG LogBufferSize )
    {
        _PrintBuffer = LogBuffer;
        _PrintBufferSize = LogBufferSize;
        LockInit();
    };

BOOLEAN LogFile::Lock()
{
#ifdef _PI_KRN
    NTSTATUS Status;
    
    Status = KeWaitForSingleObject( &_KMutex, Executive, KernelMode, FALSE, NULL );
    
    if( Status != STATUS_SUCCESS )
    {
       DbgPrintEx( DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __FUNCTION__" KeWaitForMutexObject = %x\n", Status );
            return FALSE;
    }
    
#else
    EnterCriticalSection( &_CritSec );
#endif
    return TRUE;
}

VOID __forceinline LogFile::Unlock()
{
#ifdef _PI_KRN
    KeReleaseMutex( &_KMutex, FALSE );
#else
    LeaveCriticalSection( &_CritSec );
#endif
}

VOID LogFile::LockInit()
{	
#ifdef _PI_KRN
    KeInitializeMutex( &_KMutex, 0 );
#else
    InitializeCriticalSection( &_CritSec );
#endif	
}

VOID LogFile::AddLine( __in HANDLE LogHandle, __in PCHAR Msg, __in ULONG MsgSizeInBytes )
{
#ifdef _PI_KRN
    IO_STATUS_BLOCK IoStatus;
    ZwWriteFile( LogHandle, NULL, NULL, NULL, &IoStatus, (PVOID)Msg, MsgSizeInBytes, NULL, NULL );
    ZwWriteFile( LogHandle, NULL, NULL, NULL, &IoStatus, (PVOID)"\r\n", 2, NULL, NULL );
#else
    ULONG BytesWritten;
    WriteFile( LogHandle, Msg, MsgSizeInBytes, &BytesWritten, NULL );
    WriteFile( LogHandle, "\r\n", 2, &BytesWritten, NULL );
#endif
}

VOID LogFile::Print( __in HANDLE LogHandle, __in_z __format_string PCHAR format, ... )
{
    BOOLEAN bRes;
    PCHAR pBufferEnd;

    if( NULL == _PrintBuffer )
        return;

    if( !this->Lock() )
        return;

    va_list list;
    va_start (list, format);

#ifdef _PI_KRN
    bRes = NT_SUCCESS( RtlStringCchVPrintfExA( (NTSTRSAFE_PSTR)_PrintBuffer, _PrintBufferSize, &pBufferEnd, NULL, 0, format, list ) );
#else
    bRes = SUCCEEDED( StringCchVPrintfExA( (STRSAFE_LPSTR)_PrintBuffer, _PrintBufferSize, &pBufferEnd, NULL, 0, format, list ) );
#endif
    
    va_end(list);

    if( bRes )
    {
        ULONG MsgSizeInBytes = (ULONG)(pBufferEnd - (PCHAR)_PrintBuffer);
        this->AddLine( LogHandle, _PrintBuffer, MsgSizeInBytes );
    }
    else
    {
        CHAR ErrorMsg[] = "!!!!!! PRINTING ERROR "__FUNCTION__" !!!!!!";
        this->AddLine( LogHandle, ErrorMsg, sizeof(ErrorMsg) - 1 );
        this->AddLine( LogHandle, format, (ULONG)strlen(format) );
    }

    this->Unlock();
}

//************************************
// Method:    CreateLogFile                 Function creates new empty log file it overwrites 
//                                          already existing file with same name. Must be called
//                                          on PASSIVE_LEVEL if used in KM
//                                          
// FullName:  CDbgPrint::CreateLogFile
// Access:    public 
// Returns:   HANDLE                        Return handle to log file, or NULL if failed
// Qualifier:
// Parameter: __in PWCHAR LogFolderPath     Folder path where log file will be created e.g. "C:\PI_LOGS"
// Parameter: __in PWCHAR LogFileName       Log file name without extension e.g. "pisvc_log" .log extension 
//					                        will be appended automatically e.g. "pisvc_log.log"
// Parameter: __in_opt ULONG Id             Parameter is optional and will be converter to string and
//                                          then appended to log file name e.g. "pisvc_log1234.log"
//************************************
HANDLE LogFile::Create( __in_z PWCHAR LogFolderPath, __in_z PWCHAR LogFileName, __in_opt ULONG Id )
{
    HANDLE hLog = NULL;    
    WCHAR FilePath[300];
    ULONG FileAccess = FILE_GENERIC_READ | FILE_GENERIC_WRITE;

#ifdef _PI_KRN	
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjAttr;
    UNICODE_STRING FilePath_UStr;
    IO_STATUS_BLOCK IoStatus;
    
    if (Id)
        Status = RtlStringCchPrintfW( FilePath, _countof(FilePath), L"\\??\\%s\\%s%lu.log", LogFolderPath, LogFileName, Id );
    else
        Status = RtlStringCchPrintfW( FilePath, _countof(FilePath), L"\\??\\%s\\%s.log", LogFolderPath, LogFileName);

    if( NT_SUCCESS(Status) )
    {
        RtlInitUnicodeString( &FilePath_UStr, FilePath );
        InitializeObjectAttributes( &ObjAttr, &FilePath_UStr, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL );
        
        Status = ZwCreateFile( &hLog, FileAccess, &ObjAttr, &IoStatus, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, 
            FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0 );

        if( !NT_SUCCESS(Status) )
        {
            hLog = NULL;

            DbgPrintEx( DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
                "ERROR CreateLogFile = %x\n", Status );
        }
    }
#else
    HRESULT hResult;

    if( Id )
        hResult = StringCchPrintfW( FilePath, _countof(FilePath), L"%s\\%s%lu.log", LogFolderPath, LogFileName, Id );
    else
        hResult = StringCchPrintfW( FilePath, _countof(FilePath), L"%s\\%s.log", LogFolderPath, LogFileName );

    if( SUCCEEDED(hResult) )
    {
        hLog = CreateFileW( FilePath, FileAccess, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        
        if( INVALID_HANDLE_VALUE == hLog )
            hLog = NULL;
    }
#endif
    return hLog;
}

VOID LogFile::Close( __in HANDLE LogHandle )
{
    if( NULL == LogHandle )
        return;

#ifdef _PI_KRN	
    ZwClose( LogHandle );
#else
    CloseHandle( LogHandle );
#endif
}

NAMESPACE_PI_END