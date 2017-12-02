//////////////////////////////////////////////////////////////////////////
///
/// File:           prociso.cpp
/// Author:         Volodymyr Pikhur
/// Description:    ProcessIsolator kernel driver implementation
///
//////////////////////////////////////////////////////////////////////////
#include <ntifs.h>
#include <suppress.h>

#include <pi/pi.h>
#include "pi/pi_pi.h"

PI_DEVICE_EXTENSION g_PIDevExt = { NULL };

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
__drv_functionClass(DRIVER_DISPATCH)
__drv_requiresIRQL(PASSIVE_LEVEL)
__drv_sameIRQL
NTSTATUS DispatchCreateClose( __in PDEVICE_OBJECT DeviceObject, __inout PIRP Irp )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION irpSp;
    UNREFERENCED_PARAMETER( DeviceObject );    

    DbgPrintEx(  DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__" Entered\n" );

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch ( irpSp->MajorFunction )
    {
    case IRP_MJ_CREATE:			///< Occurs when device is opened by ProcessIsolatorSvc
        {
            Status = STATUS_SUCCESS;
        }break;
    case IRP_MJ_CLOSE:			///< Occurs when ProcessIsolatorSvc called CloseHandle
        {            
            Status = PIDevExt.Pi->PI_Stop();
        }break;
    default:
        break;
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return Status;
}

// _Dispatch_type_(IRP_MJ_READ)
// __drv_functionClass(DRIVER_DISPATCH)
// __drv_requiresIRQL(PASSIVE_LEVEL)
// __drv_sameIRQL
// NTSTATUS DispatchRead( __in PDEVICE_OBJECT DeviceObject, __inout PIRP Irp )
// {
// 	NTSTATUS Status = STATUS_UNSUCCESSFUL;
// 	PIO_STACK_LOCATION irpSp;
// 	PUCHAR OutBuffer = NULL;
// 	ULONG_PTR Info = 0;
// 
// 	DbgPrintEx(  DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__" Entered\n" );
// 
// 	irpSp = IoGetCurrentIrpStackLocation(Irp);
// 
// 	if( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ) )
// 		OutBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
// 
// 	if( NULL != OutBuffer )
// 	{
// // 		ULONG Length = irpSp->Parameters.Read.Length;
// // 		ULONG64 Offset = (ULONG64)irpSp->Parameters.Read.ByteOffset.QuadPart;
// // 
// // 		if( 0 != Length )
// // 		{
// // 
// // 		}		
// 	}
// 
// 	Irp->IoStatus.Status = Status;
// 	Irp->IoStatus.Information = Info;
// 	IoCompleteRequest(Irp, IO_NO_INCREMENT);
// 	
// 	return Status;
// }

// _Dispatch_type_(IRP_MJ_WRITE)
// __drv_functionClass(DRIVER_DISPATCH)
// __drv_requiresIRQL(PASSIVE_LEVEL)
// __drv_sameIRQL
// NTSTATUS DispatchWrite( __in PDEVICE_OBJECT DeviceObject, __inout PIRP Irp )
// {	
// 	UNREFERENCED_PARAMETER( DeviceObject );
// 
// 	DbgPrintEx(  DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__" Entered\n" );
// 
// 	Irp->IoStatus.Status = STATUS_SUCCESS;
// 	Irp->IoStatus.Information = 0;
// 	IoCompleteRequest(Irp, IO_NO_INCREMENT);
// 	return STATUS_SUCCESS;
// }

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
__drv_functionClass(DRIVER_DISPATCH)
__drv_requiresIRQL(PASSIVE_LEVEL)
__drv_sameIRQL
NTSTATUS DispatchDeviceControl( __in PDEVICE_OBJECT DeviceObject, __inout PIRP Irp )
{
    ULONG IoCtl;
    UNREFERENCED_PARAMETER( DeviceObject );

    DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__" Entered\n" );
    
    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    IoCtl = irpSp->Parameters.DeviceIoControl.IoControlCode;
    
    switch( IoCtl )
    {
    case PI::PI_IOCTL_OPERATION:
        {
            ULONG RequestSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;
            PI::PPI_OPERATION_REQUEST Request = (PI::PPI_OPERATION_REQUEST)Irp->AssociatedIrp.SystemBuffer;

            if( 0 == RequestSize || NULL == Request || sizeof(*Request) != RequestSize )
                break;
            
            Irp->IoStatus.Status = PIDevExt.Pi->PI_ProcessIoctlOperation( Request );

        }break;
    default: 
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST; break;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

_Function_class_(DRIVER_UNLOAD)
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID DriverUnload( __in PDRIVER_OBJECT DriverObject )
{	
    UNREFERENCED_PARAMETER( DriverObject );

    DbgPrintEx(  DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__" Entered\n" );

    PPI_DEVICE_EXTENSION DevExt = 
        (PPI_DEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;

    if( NULL != DevExt->ServicePath.Buffer )
        RtlFreeUnicodeString( &DevExt->ServicePath );

    if( NULL != DevExt->DeviceObject )
        IoDeleteDevice( DevExt->DeviceObject );

    IoDeleteSymbolicLink( &DevExt->SymlinkName );

    delete DevExt->Pi;
}


extern "C" __drv_functionClass(DRIVER_INITIALIZE) __drv_sameIRQL NTSTATUS 
    DriverEntry( __inout PDRIVER_OBJECT DriverObject, __in PUNICODE_STRING RegistryPath )
{
    NTSTATUS Status;

    DbgPrintEx( DPFLTR_IHVDRIVER_ID,  DPFLTR_ERROR_LEVEL, __FUNCTION__" Entered\n" );

    PIDevExt.Pi = new PI::ProcessIsolator( &g_PIDevExt );

    if( NULL == PIDevExt.Pi || !PIDevExt.Pi->PI_Init() )
        return STATUS_INSUFFICIENT_RESOURCES;

    // Save service path to our driver	
    Status = RtlUpcaseUnicodeString( &PIDevExt.ServicePath, RegistryPath, TRUE );

    if( !NT_SUCCESS( Status ) )
        goto _Cleanup;

    RtlInitUnicodeString( &PIDevExt.DeviceName, L"\\Device\\ProcessIsolator" );
    RtlInitUnicodeString( &PIDevExt.SymlinkName, L"\\DosDevices\\ProcessIsolator" );    

    // Create the device object.
    if( !NT_SUCCESS( Status = IoCreateDevice( DriverObject, 0, &PIDevExt.DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &PIDevExt.DeviceObject ) ) )
        goto _Cleanup;

    // Now create the respective symbolic link object
    if( !NT_SUCCESS( Status = IoCreateSymbolicLink( &PIDevExt.SymlinkName, &PIDevExt.DeviceName ) ) )
        goto _Cleanup;

    //
    // Save our Driver Object
    //
    PIDevExt.DriverObject = DriverObject;
    
    //
    // Setup basic handlers
    //
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
    //DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    //DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
    DriverObject->DriverUnload = DriverUnload;	
    
    if( !NT_SUCCESS( Status = RtlGetVersion( &PIDevExt.OsVerInfo ) ) )
        goto _Cleanup;    

    SetFlag( PIDevExt.DeviceObject->Flags, DO_BUFFERED_IO );
    ClearFlag( PIDevExt.DeviceObject->Flags, DO_DEVICE_INITIALIZING );

    PIDevExt.DeviceObject->DeviceExtension = (PVOID)&PIDevExt;    
    
    PILog.Print( PILogHandle, __FUNCTION__" Initialization SUCCESSFUL" );

    return STATUS_SUCCESS;

_Cleanup:

    if( NULL != PIDevExt.ServicePath.Buffer )
        RtlFreeUnicodeString( &PIDevExt.ServicePath );

    if( NULL != PIDevExt.DeviceObject )
        IoDeleteDevice( PIDevExt.DeviceObject );

    IoDeleteSymbolicLink( &PIDevExt.SymlinkName );    
    
    if( NULL != PIDevExt.Pi )
    {
        delete PIDevExt.Pi;
    }

    return Status;
}