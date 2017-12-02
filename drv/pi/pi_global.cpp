#include <pi/pi.h>

//#ifdef _AMD64_
PVOID __CRTDECL operator new ( size_t iSize )
{
    PVOID result = ExAllocatePoolWithTag( NonPagedPool, iSize, PIPOOL );

    if( result )
    {
        RtlZeroMemory( result, iSize );
    }

    return result;
}

void __CRTDECL operator delete ( PVOID pVoid )
{
    if( pVoid )
        ExFreePoolWithTag( pVoid, PIPOOL );
}
//#endif // _AMD64_