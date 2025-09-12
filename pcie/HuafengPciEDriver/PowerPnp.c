#include "Driver.h"
//#include "PowerPnp.tmh"


const CHAR* apStateNames[3] =
{
	"NotStarted",
	"Deleted",
	"Started"
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HfPciE_RegisterPowerPnpEvt)
#pragma alloc_text(PAGE, HfPciE_EvtDeviceD0Entry)
#pragma alloc_text(PAGE, HfPciE_EvtDeviceD0Exit)
#pragma alloc_text(PAGE, HfPciE_EvtDevicePrepareHardware)
#pragma alloc_text(PAGE, HfPciE_EvtDeviceReleaseHardware)
#endif


// 注册PowerPnp入口
void HfPciE_RegisterPowerPnpEvt
(
	_Inout_ PWDFDEVICE_INIT pDeviceInit
)
{
	// PNP Power回调
	WDF_PNPPOWER_EVENT_CALLBACKS oPnpPowerCallbacks;

	PAGED_CODE();

	// 配置PNP POWER回调函数
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&oPnpPowerCallbacks);
	// 基本回调例程
	oPnpPowerCallbacks.EvtDevicePrepareHardware = HfPciE_EvtDevicePrepareHardware;
	oPnpPowerCallbacks.EvtDeviceReleaseHardware = HfPciE_EvtDeviceReleaseHardware;

	// 进入/退出D0状态
	oPnpPowerCallbacks.EvtDeviceD0Entry = HfPciE_EvtDeviceD0Entry;
	oPnpPowerCallbacks.EvtDeviceD0Exit = HfPciE_EvtDeviceD0Exit;


	// 注册 PNP和电源管理例程
	WdfDeviceInitSetPnpPowerEventCallbacks(pDeviceInit, &oPnpPowerCallbacks);
}



//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtDeviceD0Entry
// 访问权限:	public 
// 参数:      	IN WDFDEVICE hDevice
// 参数:      	IN WDF_POWER_DEVICE_STATE ePreviousState
// 函数说明:  	EvtDeviceD0Entry回调，当电源状态进入D0时调用
// 返回值类型:	NTSTATUS
// 返回值说明:	返回相应的状态代码(STATUS_SUCCESS 或对应的失败代码)
// IRQL:		PASSIVE_LEVEL
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_EvtDeviceD0Entry
(
	IN  WDFDEVICE hDevice,
	IN  WDF_POWER_DEVICE_STATE ePreviousState
)
{
	PDEVICE_CONTEXT pDeviceContext;

	PAGED_CODE();

	KdPrint(("HuafengPciEDriver.sys: EvtDeviceD0Entry().\n"));

	UNREFERENCED_PARAMETER(ePreviousState);
	pDeviceContext = DeviceGetContext(hDevice);

	pDeviceContext->bPowerD0State = TRUE;
	if (pDeviceContext->hIoQueue)
	{
		WdfIoQueueStart(pDeviceContext->hIoQueue);
	}

	return STATUS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtDeviceD0Exit
// 访问权限:	public 
// 参数:      	_In_ WDFDEVICE hDevice
// 参数:      	_In_ WDF_POWER_DEVICE_STATE eTargetState
// 函数说明:  	EvtDeviceD0Exit回调，当电源状态退出D0时调用
// 返回值类型:	NTSTATUS
// 返回值说明:	返回相应的状态代码(STATUS_SUCCESS 或对应的失败代码)
// IRQL:		PASSIVE_LEVEL
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_EvtDeviceD0Exit
(
	_In_ WDFDEVICE hDevice,
	_In_ WDF_POWER_DEVICE_STATE eTargetState
)
{
	PDEVICE_CONTEXT pDeviceContext;

	PAGED_CODE();
	KdPrint(("HuafengPciEDriver.sys: EvtDeviceD0Exit().\n"));

	UNREFERENCED_PARAMETER(eTargetState);
	pDeviceContext = DeviceGetContext(hDevice);

	pDeviceContext->bPowerD0State = FALSE;
	if (pDeviceContext->hIoQueue)
	{
		WdfIoQueuePurgeSynchronously(pDeviceContext->hIoQueue);
	}

	return STATUS_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtDevicePrepareHardware
// 访问权限:	public 
// 参数:      	_In_ WDFDEVICE hDevice
// 参数:      	_In_ WDFCMRESLIST hResourcesRaw
// 参数:      	_In_ WDFCMRESLIST hResourcesTranslated
// 函数说明:  	EvtDevicePrepareHardware回调，初始化硬件设备，枚举资源
// 返回值类型:	NTSTATUS
// 返回值说明:	返回相应的状态代码(STATUS_SUCCESS 或对应的失败代码)
// IRQL:		PASSIVE_LEVEL
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_EvtDevicePrepareHardware
(
	_In_ WDFDEVICE hDevice,
	_In_ WDFCMRESLIST hResourcesRaw,
	_In_ WDFCMRESLIST hResourcesTranslated
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	// 资源对象
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDescriptor = NULL;
	// 资源数量
	ULONG nResourceNumber = 0;
	// 设备描述表对象指针
	PDEVICE_CONTEXT pDeviceContext = NULL;
	ULONG nResourceIndex;


	PAGED_CODE();

	KdPrint(("HuafengPciEDriver.sys: EvtDevicePrepareHardware().\n"));

	pDeviceContext = DeviceGetContext(hDevice);
	UNREFERENCED_PARAMETER(hResourcesRaw);
	NTSTATUS status = STATUS_SUCCESS;
	// Raw resources
	ULONG interruptCount = 0;
	ULONG totalCount = WdfCmResourceListGetCount(hResourcesTranslated);
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pRawDesc = NULL;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pTransDesc = NULL;
	for (ULONG i = 0; i < totalCount; i++) {
		PCM_PARTIAL_RESOURCE_DESCRIPTOR res = WdfCmResourceListGetDescriptor(hResourcesTranslated, i);
		if ((res->Type == CmResourceTypeInterrupt )&&
			(res->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)) {
			pTransDesc = res;
			pRawDesc = WdfCmResourceListGetDescriptor(hResourcesRaw, i);
			KdPrint(("HuafengPciEDriver.sys:CmResourceTypeInterrupt  pRawDesc->u.MessageInterrupt.Raw.MessageCount: %d\n", pRawDesc->u.MessageInterrupt.Raw.MessageCount));
			if (pTransDesc && pRawDesc)
			{
				WDF_INTERRUPT_CONFIG interruptConfig;
				WDF_OBJECT_ATTRIBUTES interruptAttributes;
				WDF_INTERRUPT_CONFIG_INIT(&interruptConfig, HfPciE_EvtInterruptIsr, HfPciE_EvtInterruptDpc);
				KdPrint(("HuafengPciEDriver.sys:pTransDesc  && pRawDesc: TRUE\n"));
				// 关键：传递中断资源描述符
				interruptConfig.InterruptRaw = pRawDesc;
				interruptConfig.InterruptTranslated = pTransDesc;
				interruptConfig.PassiveHandling = FALSE; // 如需在PASSIVE_LEVEL处理可设为TRUE
				WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&interruptAttributes, INTERRUPT_CONTEXT);
				status = WdfInterruptCreate(
					hDevice,
					&interruptConfig,
					&interruptAttributes,
					&DeviceGetContext(hDevice)->hInterrupts[interruptCount]
				);

				if (!NT_SUCCESS(status)) 
				{
					KdPrint(("HuafengPciEDriver.sys:WdfInterruptCreate failed: 0x%08X\n", status));
					return status;
				}
				interruptCount++;
			}
			else 
			{
				KdPrint(("HuafengPciEDriver.sys:No interrupt resource found.\n"));
				return STATUS_DEVICE_CONFIGURATION_ERROR;
			}
		}
	}
	KdPrint(("HuafengPciEDriver.sys:MessageInterrupt Count: %d\n", interruptCount));
	// Translated resources
	nResourceNumber = WdfCmResourceListGetCount(hResourcesTranslated);
	KdPrint(("HuafengPciEDriver.sys: WdfCmResourceListGetCount(), Translated Resource number:%d.\n", nResourceNumber));
	pDeviceContext->nMemoryBarNumber = 0;
	for (nResourceIndex = 0; nResourceIndex < nResourceNumber; nResourceIndex++)
	{
		pResDescriptor = WdfCmResourceListGetDescriptor(hResourcesTranslated, nResourceIndex);

		if (pResDescriptor)
		{
			KdPrint(("HuafengPciEDriver.sys: WdfCmResourceListGetDescriptor(), ResourceID:%d.\n", pResDescriptor->Type));

			switch (pResDescriptor->Type)
			{
			case CmResourceTypeMemory:
				if (pDeviceContext->nMemoryBarNumber < HF_MAX_SUPPORT_MEMORY_BAR_NUMBER)
				{
					pDeviceContext->apMemBaseAddress[pDeviceContext->nMemoryBarNumber] = MmMapIoSpace(pResDescriptor->u.Memory.Start, pResDescriptor->u.Memory.Length, MmNonCached);
					pDeviceContext->anMemLength[pDeviceContext->nMemoryBarNumber] = pResDescriptor->u.Memory.Length;
					KdPrint(("HuafengPciEDriver.sys:CmResourceTypeMemory pResDescriptor->u.Memory.Length: %d\n", pResDescriptor->u.Memory.Length));
					pDeviceContext->nMemoryBarNumber++;
				}
				else
				{
					//TraceEvents(TRACE_LEVEL_ERROR, TRACE_POWERPNP, "MemeryBar number out of range.");
					return STATUS_DEVICE_CONFIGURATION_ERROR;
				}
				break;
			case CmResourceTypeInterrupt:
				if (pResDescriptor->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)
				{
					//TraceEvents(TRACE_LEVEL_ERROR, TRACE_POWERPNP, "Translated message interrupt Resource, Flags:%d, MessageCount:%d, vector:%d\r\n",
					//	pResDescriptor->Flags,
					//	pResDescriptor->u.MessageInterrupt.Raw.MessageCount,
					//	pResDescriptor->u.MessageInterrupt.Raw.Vector);
				}
				else
				{
					//TraceEvents(TRACE_LEVEL_ERROR, TRACE_POWERPNP, "Translated Legacy interrupt Resource, Flags:%d, Level:%d, vector:%d\r\n",
					//	pResDescriptor->Flags,
					//	pResDescriptor->u.Interrupt.Level,
					//	pResDescriptor->u.Interrupt.Vector);
				}
				pDeviceContext->oInterruptDescTranslated = *pResDescriptor;
			}
		}
	}

	if (pDeviceContext->nMemoryBarNumber == 0)
	{
		KdPrint(("HuafengPciEDriver.sys: Not find memery bar().\n"));
		return STATUS_DEVICE_CONFIGURATION_ERROR;
	}


	// 配置DMA
	//DMA
	nStatus = HfPciE_InitializeDMA(hDevice);
	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: Initialize DMA failed.\n"));
		return nStatus;
	}

	// 重置缓冲区节点


	HW_PCI_E_SET_NEW_PNP_STATE(pDeviceContext, ePNPState_Started);
	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtDeviceReleaseHardware
// 访问权限:	public 
// 参数:      	_In_ WDFDEVICE hDevice
// 参数:      	_In_ WDFCMRESLIST hResourcesTranslated
// 函数说明:  	EvtDeviceReleaseHardware回调，释放硬件对象时调用，销毁资源和缓冲区
// 返回值类型:	NTSTATUS
// 返回值说明:	
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_EvtDeviceReleaseHardware
(
	_In_ WDFDEVICE hDevice,
	_In_ WDFCMRESLIST hResourcesTranslated
)
{
	PDEVICE_CONTEXT pDeviceContext = NULL;

	PAGED_CODE();
	UNREFERENCED_PARAMETER(hResourcesTranslated);

	pDeviceContext = DeviceGetContext(hDevice);

	HW_PCI_E_SET_NEW_PNP_STATE(pDeviceContext, ePNPState_Deleted);

	for (ULONG i = 0; i  < pDeviceContext->nMemoryBarNumber; i++)
	{
		if (pDeviceContext->apMemBaseAddress[i])
		{
			MmUnmapIoSpace(pDeviceContext->apMemBaseAddress[i], pDeviceContext->anMemLength[i]);
			pDeviceContext->apMemBaseAddress[i] = NULL;
			pDeviceContext->anMemLength[i] = 0;
		}
	}
	// 关闭DMA监视线程，设置退出标志
	/*NTSTATUS status = STATUS_SUCCESS;
	pDeviceContext->bExitThread = TRUE;
	// 等待线程退出
	if (pDeviceContext->hThread)
	{
		status = KeWaitForSingleObject(
			&pDeviceContext->oThreadEvent,
			Executive,
			KernelMode,
			FALSE,
			NULL
		);

		if (!NT_SUCCESS(status))
		{
			KdPrint(("HfPciE_DeviceRelease: KeWaitForSingleObject failed, Status = 0x%x\n", status));
		}

		// 关闭线程句柄
		ObDereferenceObject(pDeviceContext->hThread);
		pDeviceContext->hThread = NULL;
	}
	*/
	//DestroyDataList(pDeviceContext);

	return STATUS_SUCCESS;
}
