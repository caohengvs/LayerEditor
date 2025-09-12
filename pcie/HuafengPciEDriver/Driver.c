/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
//#include "driver.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, HfPciE_EvtDeviceAdd)
#pragma alloc_text (PAGE, HfPciE_EvtDriverContextCleanup)
#endif

//////////////////////////////////////////////////////////////////////////
// 函数名称:	DriverEntry
// 访问权限:	public 
// 参数:      	_In_ PDRIVER_OBJECT DriverObject
//		DriverObject - represents the instance of the function driver that is loaded
//		into memory.DriverEntry must initialize members of DriverObject before it
//		returns to the caller.DriverObject is allocated by the system before the
//		driver is loaded, and it is released by the system after the system unloads
//		the function driver from memory.
//
// 参数:      	_In_ PUNICODE_STRING RegistryPath
//		RegistryPath - represents the driver specific path in the Registry.
//		The function driver can use the path to store driver related data between
//		reboots.The path does not store hardware instance specific data.
// 
// 函数说明:  	
//		DriverEntry initializes the driver and is the first routine called by the
//		system after the driver is loaded. DriverEntry specifies the other entry
//		points in the function driver, such as EvtDevice and DriverUnload.
// 返回值类型:	NTSTATUS
// 返回值说明:	
//		STATUS_SUCCESS if successful,
//		STATUS_UNSUCCESSFUL otherwise.
//////////////////////////////////////////////////////////////////////////
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    //
    // Initialize WPP Tracing
    //
    //WPP_INIT_TRACING(DriverObject, RegistryPath);

	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
	KdPrint(("HuafengPciEDriver.sys.sys: DriverEntry().\n"));

    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = HfPciE_EvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, HfPciE_EvtDeviceAdd);

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) {
        //TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        //WPP_CLEANUP(DriverObject);
        return status;
    }

    //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtDeviceAdd
// 访问权限:	public 
// 参数:      	_In_ WDFDRIVER Driver: 
//		Handle to a framework driver object created in DriverEntry
// 参数:      	_Inout_ PWDFDEVICE_INIT DeviceInit
//		Pointer to a framework-allocated WDFDEVICE_INIT structure.
// 函数说明:  	
//		EvtDeviceAdd is called by the framework in response to AddDevice
//		call from the PnP manager.We create and initialize a device object to
//		represent a new instance of the device.
// 返回值类型:	NTSTATUS
// 返回值说明:	
//////////////////////////////////////////////////////////////////////////
NTSTATUS
HfPciE_EvtDeviceAdd(
    _In_    WDFDRIVER       hDriver,
    _Inout_ PWDFDEVICE_INIT pDeviceInit
    )
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(hDriver);

    PAGED_CODE();

    //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    status = HfPciE_CreateDevice(pDeviceInit);

    //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtDriverContextCleanup
// 访问权限:	public 
// 参数:      	_In_ WDFOBJECT DriverObject: handle to a WDF Driver object.
// 函数说明:  	Free all the resources allocated in DriverEntry.
// 返回值类型:	VOID
//////////////////////////////////////////////////////////////////////////
VOID
HfPciE_EvtDriverContextCleanup(
    _In_ WDFOBJECT hDriverObject
    )
{
    UNREFERENCED_PARAMETER(hDriverObject);

    PAGED_CODE();

    //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    //
    // Stop WPP Tracing
    //
    //WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)hDriverObject));
}
