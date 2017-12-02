//////////////////////////////////////////////////////////////////////////
/// 
/// File:           pi_memory.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator memory manager header file
/// 
//////////////////////////////////////////////////////////////////////////
#pragma once
//
// #define _PI_KRN in your project to set use in kernel mode
//

NAMESPACE_PI_BEGIN

    class Memory
    {
    private:
#ifdef _PI_KRN
        static const ULONG _PoolTag = 'mmIP';
#endif
        HANDLE _ProcessHeap;
        struct
        {
            LONG Count;
            LONG MemDelta;
        } _MemInfo;
        ULONG GetMemSize( __in PVOID Ptr );
        PVOID sAlloc( __in ULONG nSizeInBytes );
        BOOLEAN sFree( __in PVOID Ptr );
        PVOID sReAlloc( __in PVOID Ptr, __in ULONG NewSize );
    public:
        Memory();
        ~Memory();
        BOOLEAN Init();
        __checkReturn
            PVOID Alloc( __in ULONG nSizeInBytes );
        BOOLEAN	Resize( __inout PVOID *Ptr, __in ULONG NewSize );
        BOOLEAN Free( __in_opt PVOID Ptr );
    };

NAMESPACE_PI_END