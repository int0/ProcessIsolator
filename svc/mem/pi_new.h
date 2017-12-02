#pragma once
extern PI::Memory piMem;
PVOID __CRTDECL operator new ( size_t iSize );
VOID __CRTDECL operator delete ( PVOID pVoid );