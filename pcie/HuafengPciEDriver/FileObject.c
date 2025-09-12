#include "driver.h"
//#include "FileObject.tmh"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, HfPciE_RegisterFileObjectEvt)
#pragma alloc_text (PAGE, HfPciE_EvtDeviceFileCreate)
#pragma alloc_text (PAGE, HfPciE_EvtFileClose)
#endif


// 注册文件对象入口
void HfPciE_RegisterFileObjectEvt(_Inout_ PWDFDEVICE_INIT pDeviceInit)
{
	// 文件访问回调实例
	WDF_FILEOBJECT_CONFIG oFileObjectConfig;

	PAGED_CODE();

	// 设置文件访问回调实例, 响应CreateFile和CloseHandle
	WDF_FILEOBJECT_CONFIG_INIT(&oFileObjectConfig,
		HfPciE_EvtDeviceFileCreate,			// EvtCreateFile
		HfPciE_EvtFileClose,				// EvtFileClos
		WDF_NO_EVENT_CALLBACK);						// EvtFileCleanup
	WdfDeviceInitSetFileObjectConfig(pDeviceInit, &oFileObjectConfig, WDF_NO_OBJECT_ATTRIBUTES);
}


VOID HfPciE_EvtDeviceFileCreate
(
	_In_ WDFDEVICE hDevice,
	_In_ WDFREQUEST hRequest,
	_In_ WDFFILEOBJECT hFileObject
)
{
	PDEVICE_CONTEXT pDeviceContext;
	NTSTATUS nStatus = STATUS_SUCCESS;

	PAGED_CODE();// 标明该例程占用分页内存，只能在PASSIVE_LEVEL中断级别调用该例程，否则会蓝屏
	
	
	UNREFERENCED_PARAMETER(hFileObject);
	pDeviceContext = DeviceGetContext(hDevice);

	if (pDeviceContext->eDevicePnPState != ePNPState_Started)
		nStatus = STATUS_NO_SUCH_DEVICE;
	else
	{
		pDeviceContext->bFileOpenState = TRUE;
	}
	//写中断地址
	KdPrint(("HuafengPciEDriver.sys: Write ISR Address.\n"));
	KdPrint(("HuafengPciEDriver.sys: Init Isr Buffer,Address:0x%lx.\n", pDeviceContext->pDmaIsrBufAddress));
	KdPrint(("HuafengPciEDriver.sys: Init Isr Buffer,PhiAddress:0x%lx.\n", pDeviceContext->oDmaIsrBufPhyAddress));
	KdPrint(("HuafengPciEDriver.sys: Init CommonBuffer B,Length:%lu.\n", pDeviceContext->nDmaIsrBufferLength));
	//BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_RGEISTER_ADDRESSA, pDeviceContext->oDmaIsrBufPhyAddress.LowPart);
	//BfPciE_WriteRegister(pDeviceContext->apMemBaseAddress[fpga_DesConf.nBarID], HF_PCIE_DMA_ISR_RGEISTER_ADDRESSB, pDeviceContext->oDmaIsrBufPhyAddress.LowPart + sizeof(unsigned short));
	KdPrint(("HuafengPciEDriver.sys: Write ISR Address OK.\n"));

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
	WdfRequestComplete(hRequest, nStatus);
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_FILEOBJECT, "%!FUNC! File Create!");
}
VOID HfPciE_EvtFileClose
(
	_In_ WDFFILEOBJECT hFileObject
)
{
	WDFDEVICE hDevice;
	PDEVICE_CONTEXT pDeviceContext;

	PAGED_CODE();

	hDevice = WdfFileObjectGetDevice(hFileObject);
	pDeviceContext = DeviceGetContext(hDevice);
	pDeviceContext->bFileOpenState = FALSE;
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_FILEOBJECT, "%!FUNC! File Close!");
}