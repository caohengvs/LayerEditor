#include "Driver.h"
//#include "dma.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, HfPciE_InitializeDMA)
#endif

// 中断监视线程。暂时不使用，先准备着，主要用于监视DMA传输状态和处理DMA传输完成事件
// 由于DMA传输完成事件是没有通过中断来处理的，而是FPGA写一个内存来实现通知上位机传输完成。
// 所以这个线程主要用于监视DMA传输状态
/*NTSTATUS
DMAThreadRoutine(
	_In_ PVOID Context
)
{
	PDEVICE_CONTEXT pDeviceContext = (PDEVICE_CONTEXT)Context;
	NTSTATUS status = STATUS_SUCCESS;
	// 执行线程任务
	KdPrint(("HuafengPciEDriver.sys:  Thread is running...\n"));
	// 线程循环
	while (TRUE)
	{
		// 检查是否需要退出线程
		if (pDeviceContext->bExitThread)
		{
			KdPrint(("HuafengPciEDriver.sys: Exiting thread.\n"));
			break;
		}
		INTERRUPT_STATUS* Status = pDeviceContext->pDmaIsrBufAddress;
		KdPrint(("HuafengPciEDriver.sys: Status isA: 0x%lx.B:0x%lx.\n", Status->DmaInterruptStatusA, Status->DmaInterruptStatusB));
		if (Status == NULL) {
			KdPrint(("HuafengPciEDriver.sys: Status is NULL.\n"));
			pDeviceContext->bExitThread = TRUE;
			continue;
		}
		if (Status->DmaInterruptStatusA != 0) {
			KdPrint(("HuafengPciEDriver.sys: ABuffer Interrupt Status True.\n"));
			// 处理BufferA中断
			if (HfPciE_AddUploadBuffer(pDeviceContext,
				pDeviceContext->pDmaCommBufAddressA,
				pDeviceContext->nDmaCommonBufferLength))
			{
				KdPrint(("HuafengPciEDriver.sys: Add Upload Buffer A Success.\n"));
			}
			else
			{
				KdPrint(("HuafengPciEDriver.sys: Add Upload Buffer A Failed.\n"));
			}
			Status->DmaInterruptStatusA = 0; // 清除中断状态
			BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_ENABLE_BUFFERA, 0x00000001);//打开BufferA空间写使能。
		}
		else if (Status->DmaInterruptStatusB != 0) {
			KdPrint(("HuafengPciEDriver.sys: BBuffer Interrupt Status True.\n"));
			// 处理BufferB中断
			if (HfPciE_AddUploadBuffer(pDeviceContext,
				pDeviceContext->pDmaCommBufAddressB,
				pDeviceContext->nDmaCommonBufferLength))
			{
				KdPrint(("HuafengPciEDriver.sys: Add Upload Buffer B Success.\n"));
			}
			else
			{
				KdPrint(("HuafengPciEDriver.sys: Add Upload Buffer B Success.\n"));
			}
			Status->DmaInterruptStatusB = 0; // 清除中断状态
			BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_ENABLE_BUFFERB, 0x00000001);//打开BufferB空间写使能。
		}
		else
		{
			// 延迟一段时间
			KeDelayExecutionThread(KernelMode, FALSE, &pDeviceContext->oThreadDelay);
		}
	}
	// 线程退出
	PsTerminateSystemThread(status);
	KdPrint(("HuafengPciEDriver.sys: thread Exitted.\n"));
	return status;


}*/


//////////////////////////////////////////////////////////////////////////
// 函数名称:	HfPciE_InitializeDMA
// 访问权限:	public 
// 参数:      	IN WDFDEVICE hDevice
// 函数说明:  	初始化DMA
// 返回值类型:	NTSTATUS
// 返回值说明:	返回相应的状态代码(STATUS_SUCCESS 或对应的失败代码)
// IRQL:		PASSIVE_LEVEL
//////////////////////////////////////////////////////////////////////////
NTSTATUS HfPciE_InitializeDMA
(
	IN WDFDEVICE hDevice
)
{
	NTSTATUS nStatus = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext;
	WDF_DMA_ENABLER_CONFIG oDmaConfig;

// 	BYTE* pCommBufferAddressA = NULL;
// 	BYTE* pCommBufferAddressB = NULL;
// 	PHYSICAL_ADDRESS oPhyAddressA;
// 	PHYSICAL_ADDRESS oPhyAddressB;

	PAGED_CODE();
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DMA, "%!FUNC! DMAInitialize entry!");
	pDeviceContext = DeviceGetContext(hDevice);

	// 设置DMA Common Buffer地址对齐为DWORD对齐
	WdfDeviceSetAlignmentRequirement(hDevice, FILE_OCTA_ALIGNMENT);


	// 创建DmaEnabler
	WDF_DMA_ENABLER_CONFIG_INIT(&oDmaConfig,
		WdfDmaProfilePacket64,
		HF_DMA_MAX_TRANSFER_LENGTH);

	// Create dma buffer level1
	nStatus = WdfDmaEnablerCreate(hDevice,
		&oDmaConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDeviceContext->hDmaEnabler);

	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: WdfDmaEnablerCreate failed, status:0x%08x.\n", nStatus));
		return nStatus;
	}

	// DMA Common buffer
	pDeviceContext->nDmaCommonBufferLength = HF_DMA_COMMON_BUFFER_LENGTH;
	pDeviceContext->nDmaIsrBufferLength = HF_DMA_DECROPTOR_BUFFER_LENGTH;

	WDF_COMMON_BUFFER_CONFIG oCommBufCfg;
	WDF_COMMON_BUFFER_CONFIG_INIT(&oCommBufCfg, FILE_256_BYTE_ALIGNMENT);
	// DMA Common Buffer A
	nStatus = WdfCommonBufferCreateWithConfig(pDeviceContext->hDmaEnabler,
		pDeviceContext->nDmaCommonBufferLength,
		&oCommBufCfg,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDeviceContext->hDmaCommBufA);

	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: WdfCommonBufferCreateA failed, status:%d.\n", nStatus));
		return nStatus;
	}
	pDeviceContext->pDmaCommBufAddressA = WdfCommonBufferGetAlignedVirtualAddress(pDeviceContext->hDmaCommBufA);
	pDeviceContext->oDmaCommBufPhyAddressA = WdfCommonBufferGetAlignedLogicalAddress(pDeviceContext->hDmaCommBufA);
	RtlZeroMemory(pDeviceContext->pDmaCommBufAddressA, pDeviceContext->nDmaCommonBufferLength);

	// DMA Common Buffer B
	nStatus = WdfCommonBufferCreateWithConfig(pDeviceContext->hDmaEnabler,
		pDeviceContext->nDmaCommonBufferLength,
		&oCommBufCfg,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDeviceContext->hDmaCommBufB);

	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: WdfCommonBufferCreateB failed, status:%d.\n", nStatus));
		return nStatus;
	}
	pDeviceContext->pDmaCommBufAddressB = WdfCommonBufferGetAlignedVirtualAddress(pDeviceContext->hDmaCommBufB);
	pDeviceContext->oDmaCommBufPhyAddressB = WdfCommonBufferGetAlignedLogicalAddress(pDeviceContext->hDmaCommBufB);
	RtlZeroMemory(pDeviceContext->pDmaCommBufAddressB, pDeviceContext->nDmaCommonBufferLength);
	KdPrint(("HuafengPciEDriver.sys: Init CommonBuffer B,Length:%lu.\n", pDeviceContext->nDmaCommonBufferLength));
	unsigned long* pUlong = (unsigned long*)pDeviceContext->pDmaCommBufAddressB; // 0x0000~0xFFFF
	unsigned long data = 0; // 0x0000~0xFFFF
	for (unsigned long index = 0; index < (pDeviceContext->nDmaCommonBufferLength) / sizeof(unsigned long); index++)
	{
		pUlong[data] = data;
		data++;
	}
	//上传缓冲区初始化
	nStatus = WdfCommonBufferCreateWithConfig(pDeviceContext->hDmaEnabler,
		pDeviceContext->nDmaCommonBufferLength* HF_DMA_UPLOAD_BUFFER_COUNT,
		&oCommBufCfg,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDeviceContext->hUploadBuffer);

	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: hUploadBuffer failed, status:%d.\n", nStatus));
		return nStatus;
	}
	pDeviceContext->pUploadBufferAddress = WdfCommonBufferGetAlignedVirtualAddress(pDeviceContext->hUploadBuffer);
	KdPrint(("HuafengPciEDriver.sys: Init UploadBuffer,Address:0x%lx.\n", pDeviceContext->pUploadBufferAddress));
	pDeviceContext->oUploadBufferPhyAddress = WdfCommonBufferGetAlignedLogicalAddress(pDeviceContext->hUploadBuffer);
	KdPrint(("HuafengPciEDriver.sys: Init UploadBuffer,PhiAddress:0x%lx.\n", pDeviceContext->oUploadBufferPhyAddress));
	RtlZeroMemory(pDeviceContext->pUploadBufferAddress, pDeviceContext->nDmaCommonBufferLength * HF_DMA_UPLOAD_BUFFER_COUNT);
	KdPrint(("HuafengPciEDriver.sys: Init UploadBuffer,Length:%lu.\n", pDeviceContext->nDmaCommonBufferLength * HF_DMA_UPLOAD_BUFFER_COUNT));
	pDeviceContext->nUploadBufferLength = pDeviceContext->nDmaCommonBufferLength * HF_DMA_UPLOAD_BUFFER_COUNT;
	for (unsigned int i = 0; i < HF_DMA_UPLOAD_BUFFER_COUNT; i++)
	{
		pDeviceContext->apUploadBuffer[i] = (PVOID)((ULONG_PTR)pDeviceContext->pUploadBufferAddress + i * pDeviceContext->nDmaCommonBufferLength);
		pDeviceContext->apUploadBufferStatus[i] = HF_BUFFER_STATUS_EMPTY; // 初始化状态为0
		pDeviceContext->apUploadBufferWritePos[i] = 0; // 初始化写位置为0
		pDeviceContext->apUploadBufferReadPos[i] = 0; // 初始化读位置为0
	}
	// DMA缓冲区初始化完成后，设置上传缓冲区索引为0
	pDeviceContext->nUploadBufferWriteIndex = 0;
	pDeviceContext->nUploadBufferReadIndex = 0;

	// 中断处理使用的DMA缓冲区
	nStatus = WdfCommonBufferCreateWithConfig(pDeviceContext->hDmaEnabler,
		pDeviceContext->nDmaIsrBufferLength,
		&oCommBufCfg,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDeviceContext->hDmaIsrBuf);

	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: WdfCommonBufferCreateB failed, status:%d.\n", nStatus));
		return nStatus;
	}
	pDeviceContext->pDmaIsrBufAddress = WdfCommonBufferGetAlignedVirtualAddress(pDeviceContext->hDmaIsrBuf);
	KdPrint(("HuafengPciEDriver.sys: Init Isr Buffer,Address:0x%lx.\n", pDeviceContext->pDmaIsrBufAddress));
	pDeviceContext->oDmaIsrBufPhyAddress = WdfCommonBufferGetAlignedLogicalAddress(pDeviceContext->hDmaIsrBuf);
	KdPrint(("HuafengPciEDriver.sys: Init Isr Buffer,PhiAddress:0x%lx.\n", pDeviceContext->oDmaIsrBufPhyAddress));
	RtlZeroMemory(pDeviceContext->pDmaIsrBufAddress, pDeviceContext->nDmaIsrBufferLength);
	KdPrint(("HuafengPciEDriver.sys: Init CommonBuffer B,Length:%lu.\n", pDeviceContext->nDmaIsrBufferLength));


	// 初始化线程相关参数,方案二，用于准备当FPGA无法单独处理DMA传输完成后发送中断时，使用线程来监视DMA传输状态
	/*pDeviceContext->hThread = NULL;
	KeInitializeEvent(&pDeviceContext->oThreadEvent, NotificationEvent, FALSE);
	pDeviceContext->oThreadDelay.QuadPart = WDF_REL_TIMEOUT_IN_MS(1); // 1秒延迟
	pDeviceContext->bExitThread = FALSE;

	// 创建系统线程
	nStatus = PsCreateSystemThread(
		&pDeviceContext->hThread,
		(ACCESS_MASK)0,
		NULL,
		NULL,
		NULL,
		DMAThreadRoutine,
		pDeviceContext
	);

	if (!NT_SUCCESS(nStatus))
	{
		KdPrint(("HuafengPciEDriver.sys: PsCreateSystemThread failed, Status = 0x%x\n", nStatus));
		return nStatus;
	}*/
	KdPrint(( "HuafengPciEDriver.sys: %!FUNC! exit."));

	return nStatus;
}
