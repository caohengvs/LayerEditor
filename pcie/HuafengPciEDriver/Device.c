/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
//#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, HfPciE_CreateDevice)
#endif

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HuafengPciEDriverCreateDevice
// 访问权限:	public 
// 函数说明:    Worker routine called to create a device and its software resources.
// 参数:      	_Inout_ PWDFDEVICE_INIT pDeviceInit
//      DeviceInit - Pointer to an opaque init structure. Memory for this
//      structure will be freed by the framework when the WdfDeviceCreate
//      succeeds.So don't access the structure after that point.
// 函数说明:  	初始化设备时被调用
// 返回值类型:	NTSTATUS
// 返回值说明:	
//////////////////////////////////////////////////////////////////////////
NTSTATUS
HfPciE_CreateDevice(
    _Inout_ PWDFDEVICE_INIT pDeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES oDeviceAttributes;
    PDEVICE_CONTEXT pDeviceContext;
    WDFDEVICE hDevice;
    NTSTATUS nStatus;

    PAGED_CODE();

    KdPrint(("HuafengPciEDriver.sys: HuafengPciEDriverCreateDevice().\n"));


	//Create context which will be written into fpga in IsrDpc
	DWORD addressList[] = { 0x00000104, 0x00000100, 0x0000010c, 0x00000108, 0x00000114, 0x00000110 };
	DWORD dwValueList[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, HF_DMA_STATUS_POSITION, HF_DMA_STATUS_POSITION };
	fpga_DesConf.nBarID = 0;
	for (int i = 0; i < 6; i++) {
		fpga_DesConf.address[i] = addressList[i];
		fpga_DesConf.dwValue[i] = dwValueList[i];
	}
	fpga_FreezeConf.nBarID = 1;
	fpga_FreezeConf.address = 0x00000040;
	fpga_FreezeConf.dwValue = 0;
	fpga_ReadyConf.nBarID = 1;
	fpga_ReadyConf.address = 0x00000000;
	fpga_ReadyConf.dwValue = 0;


    // 配置Device属性
	// 配置设备为独占模式，并且I/O类型为IODirect
	WdfDeviceInitSetExclusive(pDeviceInit, FALSE);
	WdfDeviceInitSetIoType(pDeviceInit, WdfDeviceIoDirect);

	// 注册PowerPnp入口
	HfPciE_RegisterPowerPnpEvt(pDeviceInit);

	// 注册文件对象入口
	HfPciE_RegisterFileObjectEvt(pDeviceInit);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&oDeviceAttributes, DEVICE_CONTEXT);

    nStatus = WdfDeviceCreate(&pDeviceInit, &oDeviceAttributes, &hDevice);

    if (!NT_SUCCESS(nStatus))
    {
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		//	"%!FUNC! DeviceCreate failed %!STATUS!", nStatus);
		return nStatus;

	}
	// 获取设备上下文指针
	pDeviceContext = DeviceGetContext(hDevice);

	// 初始化设备上下文
	nStatus = HfPciE_InitDeviceContext(hDevice, pDeviceContext);
	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		//	"%!FUNC! InitDeviceContext failed %!STATUS!", nStatus);
		return nStatus;
	}

	// Initialize the I/O Package and any Queues
	nStatus = HfPciE_QueueInitialize(hDevice);
	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		//	"%!FUNC! Queue Initialize failed %!STATUS!", nStatus);
		return nStatus;
	}

	// 配置中断
	nStatus = HfPciE_InitializeIsrDpc(hDevice, pDeviceContext);
	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_POWERPNP,
		//	"%!FUNC! Initialize IsrDpc failed %!STATUS!", nStatus);
		return nStatus;
	}

	// Create a device interface so that applications can find and talk to us.
	nStatus = WdfDeviceCreateDeviceInterface(
		hDevice,
		&GUID_DEVINTERFACE_HuafengPciEDriver,
		NULL // ReferenceString
	);

	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		//	"%!FUNC! WdfDeviceCreateDeviceInterface failed %!STATUS!", nStatus);
		return nStatus;
	}

	

    return nStatus;
}

// 初始化设备上下文
NTSTATUS HfPciE_InitDeviceContext
(
	IN WDFDEVICE hDevice,
	_Inout_ PDEVICE_CONTEXT pDeviceContext
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	WDF_OBJECT_ATTRIBUTES oAttributes;

	// 状态相关
	pDeviceContext->eDevicePnPState = ePNPState_NotStarted;
	pDeviceContext->ePreviousPnPState = ePNPState_NotStarted;
	pDeviceContext->bPowerD0State = FALSE;
	pDeviceContext->bFileOpenState = FALSE;


	// 资源
	for (int i = 0; i  < HF_MAX_SUPPORT_MEMORY_BAR_NUMBER; i++)
	{
		pDeviceContext->apMemBaseAddress[i] = NULL;
		pDeviceContext->anMemLength[i] = 0;
	}
	pDeviceContext->nMemoryBarNumber = 0;

	// I/O操作相关
	pDeviceContext->hIoQueue = NULL;

	// IO操作的自旋锁
	WDF_OBJECT_ATTRIBUTES_INIT(&oAttributes);
	oAttributes.ParentObject = hDevice;
	nStatus = WdfSpinLockCreate(&oAttributes, &pDeviceContext->hIoSpinLock);
	if (STATUS_SUCCESS != nStatus)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		//	"%!FUNC! WdfSpinLockCreate failed %!STATUS!", nStatus);
		return nStatus;
	}

	// 中断相关
	pDeviceContext->hInterrupt = NULL;

	// DMA相关
	HfPciE_DoSetDmaAccess(hDevice, eHfPciE_DMAAccess_DDR_RW);

	// Common buffer
	pDeviceContext->hDmaCommBufA = NULL;
	pDeviceContext->hDmaCommBufB = NULL;
	pDeviceContext->pDmaCommBufAddressA = NULL;
	pDeviceContext->pDmaCommBufAddressB = NULL;
	pDeviceContext->oDmaCommBufPhyAddressA.QuadPart = 0;
	pDeviceContext->oDmaCommBufPhyAddressB.QuadPart = 0;
	pDeviceContext->nDmaCommonBufferLength = 0;

	// DataPoolDpcList init
	KeInitializeSpinLock(&pDeviceContext->spinLockCommBufferA);
	KeInitializeSpinLock(&pDeviceContext->spinLockCommBufferB);
	return nStatus;
}