#include <ntifs.h>
#include "Driver.h"
//#include "IsrDpc.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, HfPciE_InitializeIsrDpc)
#endif

void printSystemTime() {
	LARGE_INTEGER systemTime;
	ULONG secondsSince1970;
	KeQuerySystemTime(&systemTime);
	RtlTimeToSecondsSince1970(&systemTime, &secondsSince1970);
	KdPrint(("%lu", secondsSince1970));
}

void startNextTransfer(PDEVICE_CONTEXT pDeviceContext, PHYSICAL_ADDRESS oDmaDmaDescriptorPhyAddress) {
	//read 0x40, freeze register
	/*ULONG freezeFlag = 0;
	BfPciE_ReadRegister(pDeviceContext->apMemBaseAddress[fpga_FreezeConf.nBarID],
		fpga_FreezeConf.address,
		&freezeFlag);
	if (freezeFlag == 1) {
		KdPrint(("fpga freeze.\n"));
		return;
	}*/
	//write fpga register, configure dma descriptor
	for (int i = 0; i < 6; i++) {
		//dwAddr == 256 == 0x00000100
		if (fpga_DesConf.address[i] == 256) {
			fpga_DesConf.dwValue[i] = (DWORD)(oDmaDmaDescriptorPhyAddress.QuadPart);
		}
		//KdPrint(("Last read nBarID is %d, address is 0x%08x, value is 0x%08x\n", fpga_DesConf.nBarID, fpga_DesConf.address[i], fpga_DesConf.dwValue[i]));
		BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID],
			fpga_DesConf.address[i],
			fpga_DesConf.dwValue[i]);
	}
}

NTSTATUS HfPciE_InitializeIsrDpc
(
	_In_ WDFDEVICE hDevice,
	_Inout_ PDEVICE_CONTEXT pDeviceContext
)
{
	NTSTATUS nStatus= STATUS_SUCCESS;
	// 中断配置
	//WDF_INTERRUPT_CONFIG oInterruptConfig;
	// 中断对象属性
	//WDF_OBJECT_ATTRIBUTES oInterruptAttributes;
	// 中断上下文对象属性
	//PINTERRUPT_CONTEXT pInterruptContext;
	UNREFERENCED_PARAMETER(hDevice);
	UNREFERENCED_PARAMETER(pDeviceContext);
	PAGED_CODE();// 标明该例程占用分页内存，只能在PASSIVE_LEVEL中断级别调用该例程，否则会蓝屏

	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ISRDPC, "%!FUNC! Entry.");
	/*KdPrint(("HuafengPciEDriver.sys:Entry.\n"));
	// 创建中断对象
	//设置中断服务例程和延迟过程调用
	WDF_INTERRUPT_CONFIG_INIT(
		&oInterruptConfig,
		HfPciE_EvtInterruptIsr,
		HfPciE_EvtInterruptDpc
	);

	oInterruptConfig.EvtInterruptEnable = HfPciE_EvtInterruptEnable;
	oInterruptConfig.EvtInterruptDisable = HfPciE_EvtInterruptDisable;
	// 中断对象属性
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&oInterruptAttributes, INTERRUPT_CONTEXT);

	//创建中断对象
	nStatus = WdfInterruptCreate(hDevice,
		&oInterruptConfig,
		&oInterruptAttributes,
		&pDeviceContext->hInterrupt);

	if (!NT_SUCCESS(nStatus))
	{
		//TraceEvents(TRACE_LEVEL_ERROR, TRACE_ISRDPC, "%!FUNC! InterruptCreate failed %!STATUS!", nStatus);
		KdPrint(("HuafengPciEDriver.sys:InterruptCreate failed! nStatus is 0x%lu\n", nStatus));
		return nStatus;
	}
	*/
	//pInterruptContext = InterruptGetContext(pDeviceContext->hInterrupt);
	//pInterruptContext->bInterruptEnable = TRUE;
	//pInterruptContext->bDmaInterruptHandlerA = 0;
	//pInterruptContext->bDmaInterruptHandlerB = 0;
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ISRDPC, "%!FUNC! exit.");
	//KdPrint(("HuafengPciEDriver.sys:HfPciE_InitializeIsrDpc exit.\n"));
	return nStatus;
}


BOOLEAN HfPciE_AddUploadBuffer
(
	IN PDEVICE_CONTEXT pDeviceContext,							// 设备对象句柄
	IN PVOID pUploadBufferAddress,					// 上传缓冲区地址
	IN ULONG nUploadBufferLength					// 上传缓冲区长度
)
{
	// 检查上传缓冲区地址和长度是否有效
	if (pUploadBufferAddress == NULL || nUploadBufferLength == 0)
	{
		KdPrint(("HuafengPciEDriver.sys: Invalid upload buffer address or length.\n"));
		return FALSE;
	}
	// 检查上传缓冲区长度是否小于等于DMA公共缓冲区长度
	if (nUploadBufferLength > pDeviceContext->nDmaCommonBufferLength)
	{
		KdPrint(("HuafengPciEDriver.sys: Upload buffer length exceeds common buffer length.\n"));
		return FALSE;
	}
	// 检查上传缓冲区索引是否在有效范围内
	if (pDeviceContext->nUploadBufferWriteIndex >= HF_DMA_UPLOAD_BUFFER_COUNT)
	{
		KdPrint(("HuafengPciEDriver.sys: Upload buffer write index is out of range.\n"));
		return FALSE;
	}
	if(pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferWriteIndex] != HF_BUFFER_STATUS_EMPTY) {
		KdPrint(("HuafengPciEDriver.sys: Upload buffer is already in use.\n"));
		return FALSE;
	}
	else
	{
		// 设置上传缓冲区状态为1，表示正在使用
		pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferWriteIndex] = 0;
		pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferWriteIndex] = HF_BUFFER_STATUS_INUSE;
		KdPrint(("HuafengPciEDriver.sys: Upload buffer is ready for use.\n"));
		memcpy(pDeviceContext->apUploadBuffer[pDeviceContext->nUploadBufferWriteIndex], 
			pUploadBufferAddress, 
			nUploadBufferLength);
		pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferWriteIndex] = nUploadBufferLength;
		pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferWriteIndex] = 0;// 初始化读位置为0
		pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferWriteIndex] = HF_BUFFER_STATUS_FULL;
		KdPrint(("HuafengPciEDriver.sys: Upload buffer copied successfully.\n"));
		// 更新上传缓冲区写索引
		pDeviceContext->nUploadBufferWriteIndex++;
		if (pDeviceContext->nUploadBufferWriteIndex >= HF_DMA_UPLOAD_BUFFER_COUNT)
		{
			pDeviceContext->nUploadBufferWriteIndex = 0; // 如果索引超出范围，则重置为0
		}
		KdPrint(("HuafengPciEDriver.sys: Upload buffer write index updated to %lu.\n", pDeviceContext->nUploadBufferWriteIndex));
		// 如果上传缓冲区写索引等于读索引，则表示上传缓冲区已满
		
	}
	return TRUE;
}

BOOLEAN HfPciE_GetUploadBuffer
(
	IN PDEVICE_CONTEXT pDeviceContext,						
	IN void* pReadBuffer,				
	_Inout_ PULONG nReadBufferLength)
{
	// 检查读取缓冲区地址和长度是否有效
    if (pReadBuffer == NULL || *nReadBufferLength == 0)
	{
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Invalid read buffer address or length.\n"));
		return FALSE;
	}
	// 检查读取缓冲区长度是否小于等于DMA公共缓冲区长度
	if (*nReadBufferLength > pDeviceContext->nDmaCommonBufferLength)
	{
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Read buffer length exceeds common buffer length.\n"));
		return FALSE;
	}
	// 检查上传缓冲区读索引是否在有效范围内
	if (pDeviceContext->nUploadBufferReadIndex >= HF_DMA_UPLOAD_BUFFER_COUNT)
	{
		pDeviceContext->nUploadBufferReadIndex = 0; // 如果索引超出范围，则重置为0
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read index is out of range.Reset to 0\n"));
	}
	if ((pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferReadIndex] ==HF_BUFFER_STATUS_EMPTY)||(pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferReadIndex] == HF_BUFFER_STATUS_INUSE)) {
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer is not ready for reading.\n"));
		return FALSE;
	}

	*nReadBufferLength = min(*nReadBufferLength, pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex]);

	// 从上传缓冲区中读取数据,如果读取的长度大于上传缓冲区剩余的数据长度，则读取剩余的数据并获取下一个上传缓冲区以补全
    if (*nReadBufferLength <= (pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex]-pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex]))
	{
		memcpy(pReadBuffer,
			((char*)pDeviceContext->apUploadBuffer[pDeviceContext->nUploadBufferReadIndex])+ pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex],
              *nReadBufferLength);
        pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] += *nReadBufferLength;
		if(pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] >= pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex])
		{
			pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferReadIndex] = HF_BUFFER_STATUS_EMPTY; // 设置上传缓冲区状态为0，表示已读取,可以重新使用
			pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex] = 0; // 重置读位置
			pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] = 0; // 重置读位置
			pDeviceContext->nUploadBufferReadIndex++;
			KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read position reset.\n"));
		}
		else
		{
            KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read position updated to %lu.\n", pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex]));
		}
	}// 如果读取的长度大于上传缓冲区剩余的数据长度，则读取剩余的数据并获取下一个上传缓冲区以补全
	else if(pDeviceContext->apUploadBufferStatus[((pDeviceContext->nUploadBufferReadIndex+1)% HF_DMA_UPLOAD_BUFFER_COUNT)] == HF_BUFFER_STATUS_INUSE)
	{
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer is in use, cannot read.\n"));
		return FALSE;
	}
    else if(pDeviceContext->apUploadBufferStatus[((pDeviceContext->nUploadBufferReadIndex + 1) % HF_DMA_UPLOAD_BUFFER_COUNT)] == HF_BUFFER_STATUS_EMPTY)
	{ 
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Next upload buffer is empty, cannot read.\n"));
		return FALSE;
	}
	else//当前缓冲区不足以读取所需长度的数据，但下一个缓冲区可以补全。则读取当前缓冲区剩余的数据并获取下一个上传缓冲区以补全。否则不进行读取。
	{
		ULONG nReadLength = pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex]-pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex];
		if (nReadLength > *nReadBufferLength)
		{
			KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Cannot read more than %lu bytes.\n", *nReadBufferLength));
			return FALSE;
		}
		memcpy(pReadBuffer,
			((char*)pDeviceContext->apUploadBuffer[pDeviceContext->nUploadBufferReadIndex]) + pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex],
			nReadLength);
		pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] += nReadLength;
		pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferReadIndex] = HF_BUFFER_STATUS_EMPTY; // 设置上传缓冲区状态为0，表示已读取,可以重新使用
		pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex] = 0; // 重置写位置
		pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] = 0; // 重置读位置
		pDeviceContext->nUploadBufferReadIndex++;
		KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read position reset.\n"));
		// 检查上传缓冲区读索引是否在有效范围内
		if (pDeviceContext->nUploadBufferReadIndex >= HF_DMA_UPLOAD_BUFFER_COUNT)
		{
			pDeviceContext->nUploadBufferReadIndex = 0; // 如果索引超出范围，则重置为0
			KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read index is out of range.\n"));
		}
		// 如果读取的长度大于上传缓冲区剩余的数据长度，则获取下一个上传缓冲区以补全
		ULONG nRemainingLength = *nReadBufferLength - nReadLength;
        if (nRemainingLength > 0)
		{
			if (nRemainingLength > *nReadBufferLength)
			{
				KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Cannot read more than %lu bytes.\n", nRemainingLength));
				return FALSE;
			}
			KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Remaining length is %lu.\n", nRemainingLength));
			memcpy(((char*)pReadBuffer) + nReadLength,
				((char*)pDeviceContext->apUploadBuffer[pDeviceContext->nUploadBufferReadIndex]) + pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex],
				nRemainingLength);
			pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] += nRemainingLength;
			if (pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] >= pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex])
			{
				pDeviceContext->apUploadBufferStatus[pDeviceContext->nUploadBufferReadIndex] = HF_BUFFER_STATUS_EMPTY; // 设置上传缓冲区状态为0，表示已读取,可以重新使用
				pDeviceContext->apUploadBufferWritePos[pDeviceContext->nUploadBufferReadIndex] = 0; // 重置写位置
				pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex] = 0; // 重置读位置
				pDeviceContext->nUploadBufferReadIndex++;
				KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read position reset.\n"));
			}
			else
			{
				KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read position updated to %lu.\n", pDeviceContext->apUploadBufferReadPos[pDeviceContext->nUploadBufferReadIndex]));
			}
		}
	}
	KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Read buffer copied successfully.\n"));
	// 更新上传缓冲区读索引
	if (pDeviceContext->nUploadBufferReadIndex >= HF_DMA_UPLOAD_BUFFER_COUNT)
	{
		pDeviceContext->nUploadBufferReadIndex = 0; // 如果索引超出范围，则重置为0
	}
	KdPrint(("HuafengPciEDriver.sys:HfPciE_GetUploadBuffer Upload buffer read index updated to %lu.\n", pDeviceContext->nUploadBufferReadIndex));
	return TRUE;
}

// 中断响应
BOOLEAN HfPciE_EvtInterruptIsr
(
	IN WDFINTERRUPT hInterrupt,						// 中断对象句柄
	IN ULONG nMessageID								// 消息ID
)
{
	// 中断上下文对象属性
	PINTERRUPT_CONTEXT pInterruptContext;
	PDEVICE_CONTEXT pDeviceContext;
	BOOLEAN bIsMyInt=FALSE;
	KdPrint(("HuafengPciEDriver.sys:HfPciE_EvtInterruptIsr Entered!MessageID：%ld\n", nMessageID));
	UNREFERENCED_PARAMETER(nMessageID);
	pInterruptContext = InterruptGetContext(hInterrupt);
	pDeviceContext = DeviceGetContext(WdfInterruptGetDevice(hInterrupt));
	ULONG ISRValue = 0;
	BfPciE_ReadRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_RGEISTER_ADDRESS, &ISRValue);
    KdPrint(("HuafengPciEDriver.sys: ISRValue%08X.\n",ISRValue));
    ISR_FIELDS isr = HfPciE_DecodeISR(ISRValue);
    KdPrint(("HuafengPciEDriver.sys:Fpga upload data Length(Byte):%08X.\n", isr.Length));

    if (isr.FPGA2PC)
    {
        KdPrint(("HuafengPciEDriver.sys: WR ISR Enter, return.\n"));
        return FALSE;
    }
    if (isr.BufferA)
    {
        KdPrint(("HuafengPciEDriver.sys: ABuffer Interrupt Status True.\n"));
        // 处理BufferA中断
        HfPciE_AddUploadBuffer(pDeviceContext, pDeviceContext->pDmaCommBufAddressA, isr.Length);
        BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_ENABLE_BUFFERA,
                             0x00000001);  //打开BufferA空间写使能。
        KdPrint(("HuafengPciEDriver.sys: BfPciE_WriteRegister%08X[%08X].\n", HF_PCIE_DMA_ISR_ENABLE_BUFFERA,
                 0x00000001));
        bIsMyInt = TRUE;
    }
    if (isr.BufferB)
    {
        KdPrint(("HuafengPciEDriver.sys: BBuffer Interrupt Status True.\n"));
        // 处理BufferB中断
        HfPciE_AddUploadBuffer(pDeviceContext, pDeviceContext->pDmaCommBufAddressB, isr.Length);
        BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_ENABLE_BUFFERB,
                             0x00000001);  //打开BufferB空间写使能。
        KdPrint(("HuafengPciEDriver.sys: BfPciE_WriteRegister%08X[%08X].\n", HF_PCIE_DMA_ISR_ENABLE_BUFFERB,
                 0x00000001));
        bIsMyInt = TRUE;
    }
    KdPrint(("HuafengPciEDriver.sys:HfPciE_EvtInterruptIsr Exit!\n"));
	/*INTERRUPT_STATUS* Status = pDeviceContext->pDmaIsrBufAddress;
	KdPrint(("HuafengPciEDriver.sys: pDmaIsrBuf Status isA: 0x%lx.B:0x%lx.\n", Status->DmaInterruptStatusA, Status->DmaInterruptStatusB));
	if(Status== NULL) {
		KdPrint(("HuafengPciEDriver.sys: Status is NULL.\n"));
		return FALSE;
	}
	if(Status->DmaInterruptStatusA != 0) {
		KdPrint(("HuafengPciEDriver.sys: ABuffer Interrupt Status True.\n"));
		// 处理BufferA中断
		HfPciE_AddUploadBuffer(pDeviceContext, 
			pDeviceContext->pDmaCommBufAddressA, 
			pDeviceContext->nDmaCommonBufferLength);
		Status->DmaInterruptStatusA = 0; // 清除中断状态
		BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_ENABLE_BUFFERA, 0x00000001);//打开BufferA空间写使能。
		bIsMyInt = TRUE;
	}
	if (Status->DmaInterruptStatusB  != 0) {
		KdPrint(("HuafengPciEDriver.sys: BBuffer Interrupt Status True.\n"));
		// 处理BufferB中断
		HfPciE_AddUploadBuffer(pDeviceContext, 
			pDeviceContext->pDmaCommBufAddressB, 
			pDeviceContext->nDmaCommonBufferLength);
		Status->DmaInterruptStatusB = 0; // 清除中断状态
		BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_ENABLE_BUFFERB, 0x00000001);//打开BufferB空间写使能。
		bIsMyInt = TRUE;
	}
	*/
	return bIsMyInt;
}

// 中断延迟调用
VOID HfPciE_EvtInterruptDpc
(
	IN WDFINTERRUPT hInterrupt,						// 中断对象句柄
	IN WDFOBJECT hDevice							// 设备对象句柄
)
{
	UNREFERENCED_PARAMETER(hInterrupt);
	UNREFERENCED_PARAMETER(hDevice);
}

NTSTATUS HfPciE_EvtInterruptEnable
(
	IN WDFINTERRUPT hInterrupt,						// 中断对象句柄
	IN WDFDEVICE    hDevice							// 设备对象句柄
)
{
	UNREFERENCED_PARAMETER(hDevice);

	// 中断上下文对象属性
	PINTERRUPT_CONTEXT pInterruptContext;
	pInterruptContext = InterruptGetContext(hInterrupt);

	pInterruptContext->bInterruptEnable = TRUE;
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ISRDPC, "%!FUNC! Set Interrupt Enable");
	KdPrint(("HfPciE_EvtInterruptEnable exited\n"));
	return STATUS_SUCCESS;
}

NTSTATUS HfPciE_EvtInterruptDisable
(
	IN WDFINTERRUPT hInterrupt,						// 中断对象句柄
	IN WDFDEVICE    hDevice							// 设备对象句柄
)
{
	UNREFERENCED_PARAMETER(hDevice);

	// 中断上下文对象属性
	PINTERRUPT_CONTEXT pInterruptContext;
	pInterruptContext = InterruptGetContext(hInterrupt);

	pInterruptContext->bInterruptEnable = FALSE;
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ISRDPC, "%!FUNC! Set Interrupt Disabled");
	KdPrint(("HfPciE_EvtInterruptDisable exited\n"));
	return STATUS_SUCCESS;
}