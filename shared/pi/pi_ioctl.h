//////////////////////////////////////////////////////////////////////////
/// 
/// File:           pi_ioctl.h
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator IOCTL definitions header file
/// 
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "pi.h"

NAMESPACE_PI_BEGIN

#define PROCESS_ISOLATOR_DEVICE_TYPE         0x8150
#define PISOFCODE(n)                        (0x800|n)

typedef enum _PI_IOCTL
{
    PI_IOCTL_OPERATION        = CTL_CODE( PROCESS_ISOLATOR_DEVICE_TYPE, PISOFCODE(0x000), METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA ),
} PI_IOCTL, *PPI_IOCTL;

typedef enum _PI_OPERATION_TYPE
{
    UnknownOperation,
    StartProcessIsolator,                           ///< Start kernel mode PI client ( RECOMMENDED: server should be running already )
    StopProcessIsolator,                            ///< Stops kernel mode PI client
    RegisterProcess,                                ///< Adds process to isolation to list
    UnregisterProcess,                              ///< Removes Process from isolation list
    TerminateProcess,                               ///< Terminates process by id
    ReadMemory,                                     ///< Reads memory
    WriteMemory,                                    ///< Writes memory
    GetProcessList,                                 ///< Returns list of isolated processes
    MaxOperation,
} PI_OPERATION_TYPE, *PPI_OPERATION_TYPE;

typedef struct _PI_MEMORY_OPERATION
{    
    PVOID Address;                                  ///< Address of memory operation can be user or kernel address
    PVOID Buffer;                                   ///< Memory operation buffer
    ULONG BufferSize;                               ///< Memory buffer size
} PI_MEMORY_OPERATION, *PPI_MEMORY_OPERATION;

typedef struct _PI_PROCESS_OPERATION
{
    union
    {
        struct
        {
            PHANDLE Buffer;                                   ///< Pointer to buffer where to store isolated Process IDs
            ULONG Size;                                       ///< Max count of isolated Process ID's
        } List;
    };
} PI_PROCESS_OPERATION, *PPI_PROCESS_OPERATION;

typedef struct _PI_OPERATION_REQUEST
{
    PI_OPERATION_TYPE Type;                    ///< Operation request type
    HANDLE ProcessId;                          ///< Process Id of target process
    union
    {
        PI_MEMORY_OPERATION Memory;
        PI_PROCESS_OPERATION Process;
    };
} PI_OPERATION_REQUEST, *PPI_OPERATION_REQUEST;

NAMESPACE_PI_END