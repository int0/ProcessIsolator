//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_msr.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator MSR manager header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <pi/pi.h>

NAMESPACE_PI_BEGIN

class MsrManager
{
private:	
	enum class MsrIndex : ULONG;
	static ULONG64 ReadMsr( __in ULONG MsrIdx );
	static VOID WriteMsr( __in ULONG MsrIdx, __in ULONG64 Value );
public:
	static PVOID GetSysEnterHandlerAddress();
	static VOID SetSysEnterHandler( __in PVOID NewHandler );
};

NAMESPACE_PI_END