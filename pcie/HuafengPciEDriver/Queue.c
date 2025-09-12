/*++

Module Name:

	queue.c

Abstract:

	This file contains the queue entry points and callbacks.

Environment:

	Kernel-mode Driver Framework

--*/

#include "driver.h"
//#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, HfPciE_QueueInitialize)
#endif

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_QueueInitialize
// 访问权限:	public 
// 参数:      	_In_ WDFDEVICE Device
// 函数说明:  	配置驱动程序中的I/O分配回调。
// 返回值类型:	NTSTATUS
// 返回值说明:	
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_QueueInitialize
(
	_In_ WDFDEVICE hDevice
)
{
	NTSTATUS nStatus;
	PDEVICE_CONTEXT pDeviceContext;
	PQUEUE_CONTEXT pQueueContext;
	WDF_IO_QUEUE_CONFIG    oQueueConfig;
	WDF_OBJECT_ATTRIBUTES oQueueAttributes;

	PAGED_CODE();
	KdPrint(("HuafengPciEDriver.sys: QueueInitialize.\n"));
	pDeviceContext = DeviceGetContext(hDevice);
	//
	// Configure a default queue so that requests that are not
	// configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
	// other queues get dispatched here.
	//
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
		&oQueueConfig,
		WdfIoQueueDispatchParallel
	);

	oQueueConfig.EvtIoDeviceControl = HfPciE_EvtIoDeviceControl;
	oQueueConfig.EvtIoRead = HfPciE_EvtIoRead;
	oQueueConfig.EvtIoWrite = HfPciE_EvtIoWrite;
	//oQueueConfig.EvtIoStop = HfPciE_EvtIoStop;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&oQueueAttributes, QUEUE_CONTEXT);


	nStatus = WdfIoQueueCreate(
		hDevice,
		&oQueueConfig,
		&oQueueAttributes,
		&pDeviceContext->hIoQueue
	);

	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", nStatus);
		return nStatus;
	}

	pQueueContext = QueueGetContext(pDeviceContext->hIoQueue);

	pQueueContext->oWriteMemeryBar.nMemoryBarID = 0;
	pQueueContext->oWriteMemeryBar.nAddressOffset = HF_PCIE_INVALID_ADDRESS;
	pQueueContext->oWriteMemeryBar.nLength = 0;

	pQueueContext->oWriteDDR.nMemoryBarID = 0;
	pQueueContext->oWriteDDR.nAddressOffset = HF_PCIE_INVALID_ADDRESS;
	pQueueContext->oWriteDDR.nLength = 0;

	return nStatus;
}
VOID HfPciE_EvtIoRead(
	IN WDFQUEUE Queue,
	IN WDFREQUEST Request,
	IN size_t Length
)
{
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));
	PVOID pOutputBuffer = NULL;
	NTSTATUS status;
	ULONG bytesToCopy = min((ULONG)Length, pDeviceContext->nDmaCommonBufferLength);
	/*WDF_REQUEST_PARAMETERS params;
	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request, &params);
	LONGLONG offset = params.Parameters.Read.DeviceOffset;
	//暂时不用，因为DMA缓冲区是连续的，不需要偏移量
	*/
	status = WdfRequestRetrieveOutputBuffer(Request, bytesToCopy, &pOutputBuffer, NULL);
	if (!NT_SUCCESS(status)) {
		KdPrint(("HuafengPciEDriver.sys:HfPciE_EvtIoRead WdfRequestRetrieveOutputBuffer Failed."));
		WdfRequestComplete(Request, status);
		return;
	}
	KdPrint(("HuafengPciEDriver.sys: HfPciE_EvtIoRead Buffer:0x%08X;Size:0x%08X.", pOutputBuffer, bytesToCopy));
	if (!HfPciE_GetUploadBuffer(pDeviceContext, pOutputBuffer, &bytesToCopy))
	{
		KdPrint(("HuafengPciEDriver.sys:HfPciE_EvtIoRead HfPciE_GetUploadBuffer Failed."));
		WdfRequestComplete(Request, STATUS_UNSUCCESSFUL); // 通知上层失败
		return;
	}
	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bytesToCopy);
}
VOID HfPciE_EvtIoWrite(
	IN WDFQUEUE Queue,
	IN WDFREQUEST Request,
	IN size_t Length
)
{
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));
	PVOID pInputBuffer = NULL;
	NTSTATUS status;
	size_t bytesToCopy = min(Length, pDeviceContext->nDmaCommonBufferLength);
	status = WdfRequestRetrieveInputBuffer(Request, bytesToCopy, &pInputBuffer, NULL);
	if (!NT_SUCCESS(status)) {
		WdfRequestComplete(Request, status);
		return;
	}
	// 假设写入到DMA缓冲区
	RtlCopyMemory(pDeviceContext->pDmaCommBufAddressB, pInputBuffer, bytesToCopy);
	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, bytesToCopy);
}
//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_EvtIoDeviceControl
// 访问权限:	public 
// 参数:      	_In_ WDFQUEUE Queue
// 参数:      	_In_ WDFREQUEST Request
// 参数:      	_In_ size_t OutputBufferLength
// 参数:      	_In_ size_t InputBufferLength
// 参数:      	_In_ ULONG IoControlCode
// 函数说明:  	I/O分发函数
// 返回值类型:	//////////////////////////////////////////////////////////////////////////
VOID HfPciE_EvtIoDeviceControl(
	_In_ WDFQUEUE hQueue,
	_In_ WDFREQUEST hRequest,
	_In_ size_t nOutputBufferLength,
	_In_ size_t nInputBufferLength,
	_In_ ULONG nIoControlCode
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	ULONG_PTR nInformation = 0;
	BOOLEAN bCompleteWithInfo = FALSE;

	/*TraceEvents(TRACE_LEVEL_INFORMATION,
		TRACE_QUEUE,
		"%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d",
		hQueue, hRequest, (int)nOutputBufferLength, (int)nInputBufferLength, nIoControlCode);*/
	
 	switch (nIoControlCode)
 	{
	case HF_PCIE_IOCTL_INITIALIZATE:
		HfPciE_InitializeDevice(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_SOFTWARE_RESET:
		HfPciE_ResetDevice(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_READ_MEMERY_BAR:
		HfPciE_ReadMemeryBar(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_SET_WRITE_MEMERY_BAR_PARAM:
		HfPciE_SetWriteMemeryBarParam(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_WRITE_MEMERY_BAR:
		HfPciE_WriteMemeryBar(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_READ_DDR_BAR:
		HfPciE_ReadDDRBar(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_SET_WRITE_DDR_BAR_PARAM:
		HfPciE_SetWriteDDRBarParam(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_WRITE_DDR_BAR:
		HfPciE_WriteDDRBar(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_DISCARD_PACKAGES_INFO:
		HfPciE_GetDiscardPackagesNum(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_RESET_DISCARD_PACKAGES_INFO:
		HfPciE_ResetDiscardPackagesNum(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_SET_DMA_ACCESS:
		HfPciE_SetDmaAccess(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_SET_BEGIN_TX_STATE:
		HfPciE_SetTxState(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_DATA_UPLOAD_PACKAGE:
		HfPciE_GetDataUploadPackage(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_MEMORY_BARS_NUMBER:
		HfPciE_GetMemoryBarsNumber(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFA_DATA:
		HfPciE_GetPhyMemoryBufAData(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFB_DATA:
		HfPciE_GetPhyMemoryBufBData(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFA_ADDR:
		HfPciE_GetPhyMemoryBufAAddr(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFB_ADDR:
		HfPciE_GetPhyMemoryBufBAddr(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
	case HF_PCIE_IOCTL_GET_PHY_MEMORY_BUF_DATA:
		nStatus=HfPciE_GetPhyMemoryBufData(hQueue, hRequest, nOutputBufferLength, nInputBufferLength, &nInformation, &bCompleteWithInfo);
		break;
 	}
	if (bCompleteWithInfo)
		WdfRequestCompleteWithInformation(hRequest, nStatus, nInformation);
	else
		WdfRequestComplete(hRequest, nStatus);

	return;
}


//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_GetIOReqInputBuffer
// 函数作用:	获取Request的输入缓冲区
// 参数:      	IN WDFREQUEST hRequest
// 参数:      	IN size_t nMinLength
// 参数:      	IN size_t nReqBufLength
// 参数:      	OUT PVOID * pInputBuf
// 返回类型:	PVOID
// 返回值说明:	成功获取到缓冲区，并且缓冲区长度与要求一致，则返回缓冲区地
//				址，否则返回NULL
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_GetIOReqInputBuffer
(
	IN WDFREQUEST hRequest,
	IN size_t nMinLength,
	IN size_t nReqBufLength,
	OUT PVOID* pInputBuf
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	size_t nBufLength;

	if (pInputBuf == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	nStatus = WdfRequestRetrieveInputBuffer(hRequest, nMinLength, pInputBuf, &nBufLength);


	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfRequestRetrieveInputBuffer failed %!STATUS!", nStatus);
		return nStatus;
	}

	if (nReqBufLength == HF_PCIE_IO_DEFAULT_REQ_LENGTH)
	{
		if (nBufLength < nMinLength)
		{
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfRequestRetrieveInputBuffer failed, Invalid input buffer length.");
			return STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
		if (nBufLength != nReqBufLength)
		{
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfRequestRetrieveInputBuffer failed, Invalid input buffer length.");
			return STATUS_INVALID_PARAMETER;
		}
	}

	return nStatus;
}

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_GetIOReqOutputBuffer
// 函数作用:	获取Request的输出缓冲区
// 参数:      	IN WDFREQUEST hRequest
// 参数:      	IN size_t nMinLength
// 参数:      	IN size_t nReqBufLength
// 参数:      	OUT PVOID * pOutputBuf
// 返回类型:	PVOID
// 返回值说明:	成功获取到缓冲区，并且缓冲区长度与要求一致，则返回缓冲区地
//				址，否则返回NULL
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_GetIOReqOutputBuffer
(
	IN WDFREQUEST hRequest,
	IN size_t nMinLength,
	IN size_t nReqBufLength,
	OUT PVOID* pOutputBuf
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	size_t nBufLength;

	if (pOutputBuf == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	nStatus = WdfRequestRetrieveOutputBuffer(hRequest, nMinLength, pOutputBuf, &nBufLength);


	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfRequestRetrieveOutputBuffer failed %!STATUS!", nStatus);
		return nStatus;
	}

	if (nReqBufLength == HF_PCIE_IO_DEFAULT_REQ_LENGTH)
	{
		if (nBufLength < nMinLength)
		{
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfRequestRetrieveOutputBuffer failed, Invalid input buffer length.");
			return STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
		if (nBufLength != nReqBufLength)
		{
			//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfRequestRetrieveOutputBuffer failed, Invalid input buffer length.");
			return STATUS_INVALID_PARAMETER;
		}
	}

	return nStatus;
}



//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_CheckMemeryBarRWParam
// 访问权限:	public 
// 参数1:      	IN PDEVICE_CONTEXT pDeviceContext
// 参数2:      	IN PBS_X1_RW_PARAM pParam
// 函数说明:  	检查读/写MemeryBar的参数是否正确
// 返回值类型:	NTSTATUS
// 返回值说明:	参数正确，返回Successful, 否则返回相应的错误代码
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_CheckMemeryBarRWParam
(
	IN PDEVICE_CONTEXT pDeviceContext,
	IN PHF_PCIE_RW_PARAM pParam
)

{
	if (pParam == NULL)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! pParam is NULL.");
		return STATUS_INVALID_PARAMETER;
	}

	if (pParam->nMemoryBarID >= pDeviceContext->nMemoryBarNumber)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid MemoryBarID.");
		return STATUS_INVALID_PARAMETER;
	}
	if (pParam->nLength == 0 || pParam->nLength % HF_PCIE_MEMERY_BAR_UNIT != 0)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid length.");
		return STATUS_INVALID_PARAMETER;
	}
	if (pParam->nAddressOffset == HF_PCIE_INVALID_ADDRESS || pParam->nAddressOffset % HF_PCIE_MEMERY_BAR_UNIT != 0)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid Address offset.");
		return STATUS_INVALID_PARAMETER;
	}

	if (pParam->nLength + pParam->nAddressOffset >= pDeviceContext->anMemLength[pParam->nMemoryBarID])
	{
		/*TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! length out of range, Length:%d, AddressOffset:%d, MemeryBar Length;%d.",
			pParam->nLength,
			pParam->nAddressOffset,
			pDeviceContext->anMemLength[pParam->nMemoryBarID]);*/

		return STATUS_INVALID_PARAMETER;
	}

	return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_CheckDdrRWParam
// 访问权限:	public 
// 参数2:      	IN PBS_X1_RW_PARAM pParam
// 函数说明:  	检查读/写DDR的参数是否正确
// 返回值类型:	NTSTATUS
// 返回值说明:	参数正确，返回Successful, 否则返回相应的错误代码
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_CheckDdrRWParam
(
	IN PHF_PCIE_RW_PARAM pParam
)
{
	if (pParam == NULL)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! pParam is NULL.");
		return STATUS_INVALID_PARAMETER;
	}

	if (pParam->nMemoryBarID >= HF_MAX_SUPPORT_DDR_BAR_NUMBER)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid MemoryBarID.");
		return STATUS_INVALID_PARAMETER;
	}

	if (pParam->nLength == 0 || pParam->nLength % HF_PCIE_DDR_UNIT != 0)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid Ddr length.");
		return STATUS_INVALID_PARAMETER;
	}
	if (pParam->nAddressOffset == HF_PCIE_INVALID_ADDRESS || pParam->nAddressOffset % HF_PCIE_DDR_UNIT != 0)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid Ddr Address offset.");
		return STATUS_INVALID_PARAMETER;
	}

	if (pParam->nLength + pParam->nAddressOffset >= HF_PCIE_DDR_LENGTH)
	{
		/*TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Ddr length out of range, Length:%d, AddressOffset:%d.",
			pParam->nLength,
			pParam->nAddressOffset);*/

		return STATUS_INVALID_PARAMETER;
	}

	return STATUS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
// 以下定义I/O请求响应函数

// 1. HF_PCIE_IOCTL_INITIALIZATE
// 提供初始化时时操作,该操作将分配缓冲区，并复位所有寄存器
// InputBuffer:  ULONG32指针, 用于保存DataUpload链表节点的数量
// OutputBuffer: 无
NTSTATUS HfPciE_InitializeDevice
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	WDFDEVICE hDevice = NULL;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PVOID pInputBuf = NULL;
	ULONG32 nNodesNumber = 0;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	hDevice = WdfIoQueueGetDevice(hQueue);
	pDeviceContext = DeviceGetContext(hDevice);

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(ULONG), nInputBufferLength, &pInputBuf);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}

	if (pInputBuf == NULL)
	{
		KdPrint(("HuafengPciEDriver.sys: InitializeDevice() Inputbuffer is null.\n"));
		return STATUS_INVALID_PARAMETER;
	}

	nNodesNumber = *((PULONG32)pInputBuf);
	if (nNodesNumber == 0)
	{
		KdPrint(("HuafengPciEDriver.sys: Nodes number is zero.\n"));
		return STATUS_INVALID_PARAMETER;
	}

	return nStatus;
}

// 2. HF_PCIE_IOCTL_SOFTWARE_RESET
// 复位操作，复位所有寄存器，清空所有未获取的数据，将DMA设置到DDR方向
// InputBuffer: 无
// OutputBuffer:无	
NTSTATUS HfPciE_ResetDevice
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	if (HfPciE_DoSoftwareReset(WdfIoQueueGetDevice(hQueue)) == FALSE)
		nStatus = STATUS_UNSUCCESSFUL;

	return nStatus;
}

// 3. HF_PCIE_IOCTL_READ_MEMERY_BAR
// 执行从MEMERYBAR中读取内存地址的操作
// InputBuffer:HF_PCIE_RW_PARAM结构体指针，包括读取的地址和长度
// OutputBuffer:保存数据的地址指针
NTSTATUS HfPciE_ReadMemeryBar
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PVOID pInputBuf = NULL;
	PVOID pOutputBuf = NULL;
	PHF_PCIE_RW_PARAM pReadParam = NULL;

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(HF_PCIE_RW_PARAM), nInputBufferLength, &pInputBuf);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	pReadParam = (PHF_PCIE_RW_PARAM)pInputBuf;

	nStatus = HfPciE_CheckMemeryBarRWParam(pDeviceContext, pReadParam);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}

	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, pReadParam->nLength, HF_PCIE_IO_DEFAULT_REQ_LENGTH, &pOutputBuf);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	if (pOutputBuf == NULL)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Output Buffer is NULL.");
		return STATUS_INVALID_PARAMETER;
	}
	if (pReadParam->nLength > nOutputBufferLength)
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "!FUNC! Invalid read OutputBufferLength, ReqLen:%d, BufLen:%d.", pReadParam->nLength, (int)nOutputBufferLength);
		return STATUS_INVALID_PARAMETER;
	}

	if (pReadParam->nLength == 4)
	{
		BfPciE_ReadRegister(pDeviceContext->apMemBaseAddress[pReadParam->nMemoryBarID],
			pReadParam->nAddressOffset,
			(PULONG)pOutputBuf);
	}
	else
	{
		BfPciE_ReadMemeryBar(pDeviceContext->apMemBaseAddress[pReadParam->nMemoryBarID],
			pReadParam->nAddressOffset,
			(PULONG)pOutputBuf,
			pReadParam->nLength / sizeof(ULONG));
	}

	*pInformation = pReadParam->nLength;
	*pSetInfomation = TRUE;

	return nStatus;
}

// 4. HF_PCIE_IOCTL_SET_WRITE_MEMERY_BAR_PARAM
// 设置下一个DeviceIOCtrl操作将要写入的的内存地址和长度
// 由于考虑32/64位操作系统的问题，在参数结构体中尽量不使用指针型变量，因此将写入操作分解为设置参数和执行写操作两步
// 读操作完成后，将解除对其它读参数设置操作的挂起
// 寄存器地址必须4字节对齐，因此地址偏移量以及读取的长度必须是4字节的整数倍
// InputBuffer:一个HF_PCIE_RW_PARAM指针
// OutputBuffer:无
NTSTATUS HfPciE_SetWriteMemeryBarParam
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PQUEUE_CONTEXT pQueueContext = NULL;
	PVOID pInputBuf = NULL;
	PHF_PCIE_RW_PARAM pWriteParam;

	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));
	pQueueContext = QueueGetContext(hQueue);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));
	pQueueContext = QueueGetContext(hQueue);

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(HF_PCIE_RW_PARAM), nInputBufferLength, &pInputBuf);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	pWriteParam = (PHF_PCIE_RW_PARAM)pInputBuf;
	nStatus = HfPciE_CheckMemeryBarRWParam(pDeviceContext, pWriteParam);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}

	pQueueContext->oWriteMemeryBar.nMemoryBarID = pWriteParam->nMemoryBarID;
	pQueueContext->oWriteMemeryBar.nAddressOffset = pWriteParam->nAddressOffset;
	pQueueContext->oWriteMemeryBar.nLength = pWriteParam->nLength;

	return nStatus;
}


// 5. HF_PCIE_IOCTL_WRITE_MEMERY_BAR
// 执行向MEMERYBAR中写数据的操作
// 该操作必须在HF_PCIE_IOCTL_SET_WRITE_MEMERY_BAR_PARAM执行成功后执行，否则将返回失败
// InputBuffer:要写入的数据的地址指针
// OutputBuffer:无
NTSTATUS HfPciE_WriteMemeryBar
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PQUEUE_CONTEXT pQueueContext = NULL;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PVOID pInputBuf = NULL;

	UNREFERENCED_PARAMETER(nOutputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));
	pQueueContext = QueueGetContext(hQueue);

	if (pQueueContext->oWriteMemeryBar.nAddressOffset == HF_PCIE_INVALID_ADDRESS || pQueueContext->oWriteMemeryBar.nLength == 0)
	{
		return STATUS_INVALID_PARAMETER;
	}
	if (pQueueContext->oWriteMemeryBar.nLength > nInputBufferLength)
	{
		return STATUS_INVALID_PARAMETER;
	}


	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, pQueueContext->oWriteMemeryBar.nLength, nInputBufferLength, &pInputBuf);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	
	// 当前前端逻辑只支持按DWORD读/写这一种模式
	if (pQueueContext->oWriteMemeryBar.nLength == 4)
	{
		BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[pQueueContext->oWriteMemeryBar.nMemoryBarID],
			pQueueContext->oWriteMemeryBar.nAddressOffset,
			*((PULONG)pInputBuf));
	}
	else
	{
		BfPciE_WriteMemeryBar(pDeviceContext->apMemBaseAddress[pQueueContext->oWriteMemeryBar.nMemoryBarID],
			pQueueContext->oWriteMemeryBar.nAddressOffset,
			(PULONG)pInputBuf,
			pQueueContext->oWriteMemeryBar.nLength / sizeof(ULONG));
	}

	*pInformation = pQueueContext->oWriteMemeryBar.nLength;
	*pSetInfomation = TRUE;

	pQueueContext->oWriteMemeryBar.nLength = 0;
	pQueueContext->oWriteMemeryBar.nAddressOffset = HF_PCIE_INVALID_ADDRESS;

	return nStatus;
}


// 6.HF_PCIE_IOCTL_READ_DDR_BAR
// 执行从DDR BAR中读取内存地址的操作
// InputBuffer:HF_PCIE_RW_PARAM结构体指针，包括读取的地址和长度
// OutputBuffer:保存数据的地址指针
NTSTATUS HfPciE_ReadDDRBar
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 7.HF_PCIE_IOCTL_SET_WRITE_DDR_BAR_PARAM
// 设置写入DDR BAR的参数
// InputBuffer:一个HF_PCIE_RW_PARAM指针
// OutputBuffer:无
NTSTATUS HfPciE_SetWriteDDRBarParam
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 8. HF_PCIE_IOCTL_WRITE_DDR_BAR
// 通过硬件的DMA执行向DDR BAR中写数据的操作
// 该操作必须在HF_PCIE_IOCTL_SET_WRITE_DDR_BAR_PARAM执行成功后执行，否则将返回失败
// InputBuffer:要写入的数据的地址指针
// OutputBuffer:无
NTSTATUS HfPciE_WriteDDRBar
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 9. HF_PCIE_IOCTL_GET_DISCARD_PACKAGES_INFO
// 获取被丢弃的线数据包的数量
// InputBuffer:无
// OutputBuffer:HF_PCIE_DISCARD_PACKAGE型的指针
NTSTATUS HfPciE_GetDiscardPackagesNum
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 10. HF_PCIE_IOCTL_RESET_DISCARD_PACKAGES_INFO
// 清除被丢弃的线数据包的计数
// InputBuffer:无
// OutputBuffer:无
NTSTATUS HfPciE_ResetDiscardPackagesNum
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 11.HF_PCIE_IOCTL_SET_DMA_ACCESS
// 设置DMA的方向
// InputBuffer:EHfPciE_DmaAccess
// OutputBuffer:无
NTSTATUS HfPciE_SetDmaAccess
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PVOID pInputBuf = NULL;
	EHfPciE_DmaAccess eDmaAccess;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(ULONG32), nInputBufferLength, &pInputBuf);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	if (pInputBuf == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}
	eDmaAccess = *((EHfPciE_DmaAccess*)pInputBuf);

	return nStatus;
}

// 12. HF_PCIE_IOCTL_SET_BEGIN_TX_STATE
// 设置开始/停止扫描
// InputBuffer:ULONG32，
// OutputBuffer:无
NTSTATUS HfPciE_SetTxState
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 13. HF_PCIE_IOCTL_GET_DATA_UPLOAD_PACKAGE
// 获取线数据包数据包
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetDataUploadPackage
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(hQueue);
	UNREFERENCED_PARAMETER(hRequest);
	UNREFERENCED_PARAMETER(nOutputBufferLength);
	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(pInformation);
	UNREFERENCED_PARAMETER(pSetInfomation);

	return nStatus;
}

// 14. HF_PCIE_IOCTL_GET_MEMORY_BARS_NUMBER
// 获取MemoryBar的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetMemoryBarsNumber
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PULONG32 pNumber = NULL;
	PVOID pOutputBuffer;

	UNREFERENCED_PARAMETER(nInputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));
	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, sizeof(ULONG32), nOutputBufferLength, &pOutputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	if (pOutputBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	pNumber = (PULONG32)pOutputBuffer;
	*pNumber = pDeviceContext->nMemoryBarNumber;

	*pInformation = sizeof(ULONG32);
	*pSetInfomation = TRUE;

	return nStatus;
}


// 15. HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFA_DATA
// 获取MemoryBar的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetPhyMemoryBufAData
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	KdPrint(("Enter function %s\n", __FUNCTION__));
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	ULONG32 nCopySize = 0;
	PVOID pInputBuffer;
	PVOID pOutputBuffer;

	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(nOutputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(ULONG32), nInputBufferLength, &pInputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	nCopySize = *((PULONG32)pInputBuffer);
	if(nCopySize > HF_DMA_COMMON_BUFFER_LENGTH)
	{
		return STATUS_INVALID_PARAMETER;
	}

	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, nCopySize, nOutputBufferLength, &pOutputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}


	if (pOutputBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	memcpy(pOutputBuffer, pDeviceContext->pDmaCommBufAddressA, nCopySize);

	*pInformation = nCopySize;
	*pSetInfomation = TRUE;

	return nStatus;
}

// 16. HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFB_DATA
// 获取MemoryBar的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetPhyMemoryBufBData
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	ULONG32 nCopySize = 0;
	PVOID pInputBuffer;
	PVOID pOutputBuffer;

	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(nOutputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(ULONG32), nInputBufferLength, &pInputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	nCopySize = *((PULONG32)pInputBuffer);
	if (nCopySize > HF_DMA_COMMON_BUFFER_LENGTH)
	{
		return STATUS_INVALID_PARAMETER;
	}

	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, nCopySize, nOutputBufferLength, &pOutputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}


	if (pOutputBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	memcpy(pOutputBuffer, pDeviceContext->pDmaCommBufAddressB, nCopySize);

	*pInformation = nCopySize;
	*pSetInfomation = TRUE;

	return nStatus;
}

// 17. HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFA_ADDR
// 获取MemoryBar的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetPhyMemoryBufAAddr
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PHYSICAL_ADDRESS* pPhyAddress = NULL;
	PVOID pOutputBuffer;

	UNREFERENCED_PARAMETER(nInputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));
	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, sizeof(PHYSICAL_ADDRESS), nOutputBufferLength, &pOutputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	if (pOutputBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	pPhyAddress = (PHYSICAL_ADDRESS *)pOutputBuffer;
	pPhyAddress->QuadPart = pDeviceContext->oDmaCommBufPhyAddressA.QuadPart;

	*pInformation = sizeof(PHYSICAL_ADDRESS);
	*pSetInfomation = TRUE;

	return nStatus;
}


// 18. HF_PCIE_IOCTL_GET_PHY_MEMORY_BUFB_ADDR
// 获取MemoryBar的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetPhyMemoryBufBAddr
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	PHYSICAL_ADDRESS* pPhyAddress = NULL;
	PVOID pOutputBuffer;

	UNREFERENCED_PARAMETER(nInputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));
	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, sizeof(PHYSICAL_ADDRESS), nOutputBufferLength, &pOutputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	if (pOutputBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	pPhyAddress = (PHYSICAL_ADDRESS*)pOutputBuffer;
	pPhyAddress->QuadPart = pDeviceContext->oDmaCommBufPhyAddressB.QuadPart;

	*pInformation = sizeof(PHYSICAL_ADDRESS);
	*pSetInfomation = TRUE;

	return nStatus;
}

// 19. HF_PCIE_IOCTL_GET_PHY_MEMORY_BUF_DATA
// 获取MemoryBar的数量
// InputBuffer:无
// OutputBuffer:缓冲区地址
NTSTATUS HfPciE_GetPhyMemoryBufData
(
	IN WDFQUEUE hQueue,
	IN WDFREQUEST hRequest,
	IN size_t nOutputBufferLength,
	IN size_t nInputBufferLength,
	OUT PULONG_PTR pInformation,
	OUT PBOOLEAN pSetInfomation
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext = NULL;
	ULONG32 nCopySize = 0;
	PVOID pInputBuffer;
	PVOID pOutputBuffer;

	UNREFERENCED_PARAMETER(nInputBufferLength);
	UNREFERENCED_PARAMETER(nOutputBufferLength);

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(hQueue));

	nStatus = HfPciE_GetIOReqInputBuffer(hRequest, sizeof(ULONG32), nInputBufferLength, &pInputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	nCopySize = *((PULONG32)pInputBuffer);
	if (nCopySize > HF_DMA_COMMON_BUFFER_LENGTH)
	{
		return STATUS_INVALID_PARAMETER;
	}

	nStatus = HfPciE_GetIOReqOutputBuffer(hRequest, nCopySize, nOutputBufferLength, &pOutputBuffer);
	if (!NT_SUCCESS(nStatus))
	{
		return nStatus;
	}
	if (pOutputBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}

	ULONG bytesToCopy = min((ULONG)nOutputBufferLength, pDeviceContext->nDmaCommonBufferLength);
	KdPrint(("HuafengPciEDriver.sys: HfPciE_EvtIoRead Buffer:%p;Size:%u.", pOutputBuffer, bytesToCopy));
	if (!HfPciE_GetUploadBuffer(pDeviceContext, pOutputBuffer, &bytesToCopy))
	{
		KdPrint(("HuafengPciEDriver.sys: HfPciE_EvtIoRead GetUploadBuffer failed."));
		*pInformation = 0;
		*pSetInfomation = TRUE;
		return STATUS_UNSUCCESSFUL;
	}
	//*pInformation = nCopySize;
    *pInformation = bytesToCopy;
	*pSetInfomation = TRUE;

	return nStatus;
}