//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_cpu.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator CPU related functions header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <pi/pi.h>

NAMESPACE_PI_BEGIN

typedef VOID ( *CPU_RUN_CALLBACK)( __in_opt ULONG CpuNumber, __in_opt PVOID Context );

class CPUManager
{
public:
	static ULONG GetNumberofActiveProcessors();
	static VOID RunThreadOnSingleCpu( __out PKAFFINITY pAffinity );
	static VOID RevertThreadAffinity( __in KAFFINITY Affinity );
	static VOID RunCallbackOnEveryCpu( CPU_RUN_CALLBACK Callback, PVOID CallbackContext );
};

NAMESPACE_PI_END