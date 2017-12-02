//////////////////////////////////////////////////////////////////////////
/// 
/// File:           pi_memory.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator memory manager implementation 
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>

NAMESPACE_PI_BEGIN

    Memory::Memory()
    {
        memset( &_MemInfo, 0, sizeof(_MemInfo) );
    }

    Memory::~Memory()
    {
        if( 0 != _MemInfo.Count )
        {
#ifdef _PI_KRN
#else
#endif
            __debugbreak();     ///< !!! CRASH !!! YOU SHOULD FIX ALL MEM LEAKS
        }
    }

    BOOLEAN Memory::Init()
    {
#ifdef _PI_KRN
#else
        if( NULL == ( _ProcessHeap = GetProcessHeap() ) )
            return FALSE;
#endif
        return TRUE;
    }

    PVOID Memory::sAlloc( __in ULONG nSizeInBytes )
    {
        PVOID Ptr;

#ifdef _PI_KRN
        Ptr = ExAllocatePoolWithTag( NonPagedPool, nSizeInBytes + sizeof(ULONG), _PoolTag );
#else
        Ptr = HeapAlloc( _ProcessHeap, 0, nSizeInBytes + sizeof(ULONG) );
#endif

        if( NULL != Ptr )
        {
            PULONG pSize = (PULONG)Ptr;
            pSize[0] = nSizeInBytes;
            return (PVOID)&pSize[1];
        }
        return NULL;
    }

    BOOLEAN Memory::sFree( __in PVOID Ptr )
    {	
        PVOID pPtr = (PVOID)((PUCHAR)Ptr-sizeof(ULONG));

#ifdef _PI_KRN
        ExFreePoolWithTag( pPtr, _PoolTag );
        return TRUE;
#else
        return HeapFree( _ProcessHeap, 0, pPtr );
#endif
    }

    ULONG __forceinline Memory::GetMemSize( __in PVOID Ptr )
    {
        return *(PULONG)((PUCHAR)Ptr-sizeof(ULONG));
    }

    PVOID Memory::sReAlloc( __in PVOID Ptr, __in ULONG NewSize )
    {
        PVOID NewPtr;

        if( 0 == NewSize )
            return NULL;

#ifdef _PI_KRN
        NewPtr = ExAllocatePoolWithTag( NonPagedPool, NewSize + sizeof(ULONG), _PoolTag );
        if( NULL != NewPtr )
        {
            ULONG OldSize = GetMemSize( Ptr );
            PULONG pSize = (PULONG)NewPtr;
            pSize[0] = NewSize;
            RtlCopyMemory( (PVOID)&pSize[1], Ptr, OldSize );
            ExFreePoolWithTag( (PVOID)((PUCHAR)Ptr-sizeof(ULONG)), _PoolTag );
            return (PVOID)&pSize[1];
        }	
#else
        NewPtr = HeapReAlloc( _ProcessHeap, 0, (PVOID)((PUCHAR)Ptr-sizeof(ULONG)), NewSize + sizeof(ULONG) );
        if( NULL != NewPtr )
        {
            PULONG pSize = (PULONG)NewPtr;
            pSize[0] = NewSize;
            return (PVOID)&pSize[1];
        }
#endif
        return NULL;
    }

    __checkReturn
    PVOID Memory::Alloc( __in ULONG nSizeInBytes )
    {
        PVOID Ptr;

        if( nSizeInBytes == 0 )
            return NULL;

        if( NULL != (Ptr = sAlloc( nSizeInBytes )))	
        {
            _InterlockedIncrement( &_MemInfo.Count );
            _InterlockedExchangeAdd( &_MemInfo.MemDelta, nSizeInBytes ); 
        }
        else
        {
            // log?
        }

        return Ptr;
    }

    BOOLEAN Memory::Free( __in_opt PVOID Ptr )
    {
        ULONG MemSize;

        if( NULL == Ptr )
            return FALSE;

        MemSize = this->GetMemSize( Ptr );

        if( sFree( Ptr ) )
        {
            _InterlockedDecrement( &_MemInfo.Count );
            _InterlockedExchangeAdd( &_MemInfo.MemDelta, -((LONG)MemSize) );
            return TRUE;
        }
        else
        {
            __debugbreak();
        }

        return FALSE;
    }

    BOOLEAN Memory::Resize( __inout PVOID *Ptr, __in ULONG NewSize )
    {	
        PVOID NewPtr;

        if( NULL == *Ptr || 0 == NewSize )
            return false;

        ULONG OldSize = GetMemSize( *Ptr );

        if( NULL == ( NewPtr = sReAlloc( *Ptr, NewSize ) ) )
            return FALSE;
        
        _InterlockedExchangeAdd( &_MemInfo.MemDelta, -((LONG)OldSize) );
        _InterlockedExchangeAdd( &_MemInfo.MemDelta, NewSize );


        *Ptr = NewPtr;

        return TRUE;
    }

NAMESPACE_PI_END
