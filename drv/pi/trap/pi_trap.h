//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_trap.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator windows kernel trap header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once

/// <summary>
/// Holds list of isolated processes used by system service handler
/// </summary>
extern "C" PI::PI_PROCESS_LIST g_PIProcessList;

#ifdef _AMD64_
extern "C" PVOID NT_KiSystemCall64;			        ///< Address of original KiSystemCall64
extern "C" VOID PI_KiSystemCall64();		        ///< SYSENTER Hook
extern "C" BOOLEAN PI_KiSystemCall64_Handler( __in PI::PREG64_CONTEXT Context );
#else
extern "C" PVOID NT_FastCallEntry;                  ///< Address of original FastCallEntry
extern "C" VOID PI_FastCallEntry();                 ///< FastCallEntry hook
extern "C" BOOLEAN PI_KiFastCallEntry_Handler( __in PI::PREG32_CONTEXT Context );
#endif