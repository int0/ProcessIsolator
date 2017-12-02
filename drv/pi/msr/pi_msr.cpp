//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_msr.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator MSR manager implementation
///
//////////////////////////////////////////////////////////////////////////
#include "pi_msr.h"
#include <intrin.h>

NAMESPACE_PI_BEGIN

enum class MsrManager::MsrIndex : ULONG
{
    //
    // We can modify TSC for each CPU using this MSR
    //
    IA32_MSR_TSC							= 0x00000010,		//RDTSC
    IA32_MSR_APIC_BASE						= 0x0000001b,
    IA32_MSR_MTRR_CAPABILITIES				= 0x000000fe,
    IA32_MSR_SYSENTER_CS					= 0x00000174,
    IA32_MSR_SYSENTER_ESP					= 0x00000175,
    IA32_MSR_SYSENTER_EIP					= 0x00000176,		//nt!KiFastCallEntry
    IA32_MSR_ENERGY_PERF_BIAS				= 0x000001b0,
    IA32_DEBUGCTL                           = 0x000001D9,
    IA32_MSR_MTRR_DEFAULT_TYPE				= 0x000002ff,
    IA32_MSR_MTRR_PHYSICAL_BASE_0			= 0x00000200,
    IA32_MSR_MTRR_PHYSICAL_MASK_0			= 0x00000201,
    IA32_PAT								= 0x00000277,
    IA32_VMX_BASIC_MSR_CODE					= 0x00000480,
    IA32_VMX_PINBASED_CTLS                  = 0x00000481,
    IA32_VMX_PROCBASED_CTLS                 = 0x00000482,
    IA32_VMX_EXIT_CTLS                      = 0x00000483,
    IA32_VMX_ENTRY_CTLS                     = 0x00000484,
    IA32_VMX_MISC                           = 0x00000485,
    IA32_VMX_CR0_FIXED0                     = 0x00000486,
    IA32_VMX_CR0_FIXED1                     = 0x00000487,
    IA32_VMX_CR4_FIXED0                     = 0x00000488,
    IA32_VMX_CR4_FIXED1                     = 0x00000489,
    IA32_MSR_EFER							= 0xC0000080,
    // x86_64 ONLY
    IA32_MSR_STAR							= 0xC0000081,
    IA32_MSR_LSTAR							= 0xC0000082,		//nt!KiSystemCall64
    IA32_MSR_CSTAR							= 0xC0000083,		//nt!KiSystemCall32		
    //
    // RFLAGS will be cleared using this mask ( Win7x64 = 0x4700 = 01000111 00000000 )		
    //
    IA32_MSR_FMASK							= 0xC0000084,		//Flags Mask
    IA32_MSR_FS_BASE						= 0xC0000100,
    IA32_MSR_GS_BASE						= 0xC0000101,
    IA32_MSR_KERNEL_GS_BASE					= 0xC0000102,
};

ULONG64 MsrManager::ReadMsr( __in ULONG MsrIdx )
{
    return __readmsr( MsrIdx );
}

VOID MsrManager::WriteMsr( __in ULONG MsrIdx, __in ULONG64 Value )
{
    __writemsr( MsrIdx, Value );
}

VOID MsrManager::SetSysEnterHandler( __in PVOID NewHandler )
{
#ifdef _X86_
    WriteMsr( (ULONG)MsrIndex::IA32_MSR_SYSENTER_EIP, (ULONG64)NewHandler );
#elif  _AMD64_
    WriteMsr( (ULONG)MsrIndex::IA32_MSR_LSTAR, (ULONG64)NewHandler );
#endif
}

PVOID MsrManager::GetSysEnterHandlerAddress()
{
#ifdef _X86_
    return (PVOID)ReadMsr( (ULONG)MsrIndex::IA32_MSR_SYSENTER_EIP );
#elif  _AMD64_
    return (PVOID)ReadMsr( (ULONG)MsrIndex::IA32_MSR_LSTAR );
#endif
}

NAMESPACE_PI_END