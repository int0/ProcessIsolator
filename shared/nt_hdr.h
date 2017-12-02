#pragma once
#include <windows.h>

extern "C" PVOID NTAPI RtlAllocateHeap( __in HANDLE HeapHandle, __in ULONG Flags, __in SIZE_T Size );
extern "C" BOOLEAN NTAPI RtlFreeHeap( __in HANDLE HeapHandle, __in ULONG Flags, __in PVOID BaseAddress );
extern "C" PVOID NTAPI RtlReAllocateHeap( __in HANDLE HeapHandle, __in ULONG Flags,	__in PVOID BaseAddress, __in SIZE_T Size );