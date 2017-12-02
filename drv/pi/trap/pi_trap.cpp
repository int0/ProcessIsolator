//////////////////////////////////////////////////////////////////////////
/// 
/// File:           pi_trap.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator windows kernel trap functions impl.
/// 
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>
#include "../../nt/nt_helpers.h"
#include "../pi_process.h"
#include "../pi_pi.h"


NAMESPACE_PI_BEGIN

extern "C" PI_PROCESS_LIST g_PIProcessList = { NULL };

//************************************
// Method:    SendContextToServer                   Function called by isolated thread from hooked SysEnter/SysCall handler 
//                                                  it awaits reply from server or returns access denied if message wasn't delivered
// FullName:  SendContextToServer
// Access:    public 
// Returns:   BOOLEAN                               TRUE    - Handler will pass execution to Windows kernel
//                                                  FALSE   - Handler will return to user mode with specified service status
// Qualifier:
// Parameter: __in PVOID Context                    Sends x86/x64 context to server
// Parameter: __in ULONG ServiceNumber              Sends system service number to server
// Parameter: __out PNTSTATUS ServiceStatus         Returns system service status handled by server 
//                                                  or access denied if message wasn't received
//************************************
BOOLEAN SendContextToServer( __in PVOID Context, __in ULONG ServiceNumber, __out PNTSTATUS ServiceStatus )
{
    PI_SYSENTER_REPLY Reply;
    PI_SYSENTER_REQUEST Request;

    //
    // do not execute service function if we were not able to send 
    // message to server to prevent unhandled execution and escape from isolation
    //
    *ServiceStatus = STATUS_ACCESS_DENIED;
    Reply.Execute = FALSE;

    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( ServiceNumber );
    UNREFERENCED_PARAMETER( ServiceStatus );

    HANDLE Pid = PsGetCurrentProcessId();
    HANDLE Tid = PsGetCurrentThreadId();

    Request.RegContext = Context;
    Request.ServiceNumber = ServiceNumber;

    PI::PPI_PROCESS_ENTRY Entry = PIProcessList.GetProcessEntry( PIList, Pid );

    if( NULL != Entry )
    {
        _InterlockedIncrement( &Entry->ThreadCount );

        PI::LpcClient *Client = PIClient(Entry);

        if( NULL != Client )
        {
            if( Client->SendMsgWaitReply( (PVOID)&Request, sizeof(Request), (PVOID)&Reply ) )
            {
                *ServiceStatus = Reply.Status;
            }
            else
            {
                // re-connect if connection is lost
                Reply.Execute = FALSE;
                Reply.Status = STATUS_ACCESS_DENIED;
            }
        }
        else
        {
            PILog.Print( PILogHandle, "ERROR[%-5d:%-5d]: Client is NULL", (ULONG)Pid, (ULONG)Tid );
        }

        _InterlockedDecrement( &Entry->ThreadCount );
    }
    else
    {
        NTSTATUS Status;
        ULONG ProcessImagePathSize = sizeof(UNICODE_STRING) + sizeof(WCHAR) * 512;
        PUNICODE_STRING ProcessImagePath;
        
        PILog.Print( PILogHandle, "ERROR[%-5d:%-5d]: Unregistered process", (ULONG)Pid, (ULONG)Tid );
        
        ProcessImagePath = (PUNICODE_STRING)PIMem.Alloc( ProcessImagePathSize );
        
        if( NULL != ProcessImagePath )
        {
            if( NT_SUCCESS( Status = PsGetProcessImagePath( PsGetCurrentProcess(), ProcessImagePath, &ProcessImagePathSize ) ) )
            {
                PILog.Print( PILogHandle, "ProcessImagePath: %wZ", ProcessImagePath );
            }
            else
            {
                PILog.Print( PILogHandle, "ProcessImagePath: <unable to retrive process image path> Status = %x", Status );
            }
            
            PIMem.Free( ProcessImagePath );
        }
    }

    return Reply.Execute;
}

#ifdef _AMD64_
extern "C" BOOLEAN PI_KiSystemCall64_Handler( __in PREG64_CONTEXT Context )
{
    BOOLEAN Execute;
    NTSTATUS ServiceStatus;
    
    Execute = SendContextToServer( (PVOID)Context, (ULONG)Context->Rax, &ServiceStatus );

    if( !Execute )
    {
        Context->Rax = ServiceStatus;
    }    

    return Execute;
}
#else
EXTERN_C {
BOOLEAN __stdcall PI_KiFastCallEntry_Handler( __in PREG32_CONTEXT Context )
{
    BOOLEAN Execute;
    NTSTATUS ServiceStatus;

    Execute = SendContextToServer( (PVOID)Context, Context->Eax, &ServiceStatus );

    if( !Execute )
    {
        Context->Eax = ServiceStatus;
    }

    return Execute;
}
}
#endif

NAMESPACE_PI_END