//////////////////////////////////////////////////////////////////////////
///
/// File:           pi_drv.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator device driver class header file
///
//////////////////////////////////////////////////////////////////////////
#pragma once

NAMESPACE_PI_BEGIN

class PIDrv
{
private:
    HANDLE _DrvHandle;
public:
    PIDrv();
    ~PIDrv();
    BOOLEAN Init();
    BOOLEAN SendOperationRequest( __in PPI_OPERATION_REQUEST Operation );
};

NAMESPACE_PI_END