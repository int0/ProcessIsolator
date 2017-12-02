//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_drv.cpp
/// Author:         Volodymyr Pikhur
/// Description:    Manager for ProcessIsolator device driver
///
//////////////////////////////////////////////////////////////////////////
#include <pi/pi.h>
#include "pi_drv.h"
#include <pi/pi_ioctl.h>

NAMESPACE_PI_BEGIN

PIDrv::PIDrv()
{
    _DrvHandle = INVALID_HANDLE_VALUE;
}
PIDrv::~PIDrv()
{
    if( INVALID_HANDLE_VALUE != _DrvHandle )
        CloseHandle( _DrvHandle );
}
BOOLEAN PIDrv::Init()
{
    _DrvHandle = CreateFileW( L"\\\\.\\ProcessIsolator", FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    return ( INVALID_HANDLE_VALUE != _DrvHandle );
}
BOOLEAN PIDrv::SendOperationRequest( __in PPI_OPERATION_REQUEST Operation )
{
    ULONG BytesRet;
    return DeviceIoControl( _DrvHandle, PI::PI_IOCTL_OPERATION, Operation, sizeof(*Operation), NULL, 0, &BytesRet, NULL );
}
NAMESPACE_PI_END

