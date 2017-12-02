//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_cpu.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator CPU related functions implementation
///
//////////////////////////////////////////////////////////////////////////
#include "pi_cpu.h"

NAMESPACE_PI_BEGIN

ULONG CPUManager::GetNumberofActiveProcessors()
{
	ULONG ProcessorsCount = 0;
	KAFFINITY CpuAffinity = KeQueryActiveProcessors();

	ULONG BitsCount = sizeof(CpuAffinity) * 8;

	for( ULONG i = 0; i < BitsCount; i++ )
	{
		if( ( (ULONG_PTR)1 << i ) & CpuAffinity )
			ProcessorsCount++;
	}

	return ProcessorsCount;
}

VOID CPUManager::RunThreadOnSingleCpu( __out PKAFFINITY pAffinity )
{
	ULONG n = KeGetCurrentProcessorNumber();
	KAFFINITY CpuAffinity = KeQueryActiveProcessors();

	KeSetSystemAffinityThread( CpuAffinity & ( (ULONG_PTR)1 << n ) );
	
	*pAffinity = CpuAffinity;
}

VOID CPUManager::RevertThreadAffinity( __in KAFFINITY Affinity )
{
	KeSetSystemAffinityThread( Affinity );
}

VOID CPUManager::RunCallbackOnEveryCpu( CPU_RUN_CALLBACK Callback, PVOID CallbackContext )
{
	ULONG BitsCount;
	ULONG CpuNumber = 0;	
	KAFFINITY CpuAffinity = KeQueryActiveProcessors();
	
	BitsCount = sizeof(CpuAffinity) * 8;

	for( ULONG i = 0; i < BitsCount; i++ )
	{
		KAFFINITY NewAffinity = ( (ULONG_PTR)1 << i ) & CpuAffinity;
		
		if( 0 != NewAffinity )
		{
			KeSetSystemAffinityThread( NewAffinity );

			Callback( CpuNumber, CallbackContext );

			CpuNumber++;
		}
	}
}

NAMESPACE_PI_END