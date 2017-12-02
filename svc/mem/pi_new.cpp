#include <pi/pi.h>

PI::Memory piMem;

PVOID __CRTDECL operator new ( size_t iSize )
{
    PVOID res = piMem.Alloc( (ULONG)iSize );

    if( NULL != res )
    {
        memset( res, 0, iSize );
    }

    return res;
}

void __CRTDECL operator delete ( PVOID pVoid )
{
    if( pVoid )
        piMem.Free( pVoid );
}