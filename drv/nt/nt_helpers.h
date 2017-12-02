//////////////////////////////////////////////////////////////////////////
///
/// File:           nt_helpers.h
/// Author:         Volodymyr Pikhur
/// Description:    Helpers for Windows kernel header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once
PVOID GetKeServiceDescriptorTable( __in PVOID SystemCallHandler );
PVOID GetSystemServiceAddressFromIndex( __in ULONG SvcIndex, __in PVOID SystemCallHandler );
NTSTATUS ObTerminateProcess( __in PEPROCESS Process );
NTSTATUS TerminateProcessById( __in HANDLE ProcessId );
NTSTATUS PsGetProcessImagePath( __in PEPROCESS Process, __in PUNICODE_STRING ImagePath, __inout PULONG Size );
