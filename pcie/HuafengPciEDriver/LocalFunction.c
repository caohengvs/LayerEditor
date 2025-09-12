#include "Driver.h"


// 执行软复位操作
BOOLEAN HfPciE_DoSoftwareReset(IN WDFDEVICE hDevice)
{
	PDEVICE_CONTEXT pDeviceContext;
	pDeviceContext = DeviceGetContext(hDevice);
	for (unsigned int i = 0; i < HF_DMA_UPLOAD_BUFFER_COUNT; i++)
	{
		pDeviceContext->apUploadBufferStatus[i] = HF_BUFFER_STATUS_EMPTY; // 初始化状态为0
		pDeviceContext->apUploadBufferWritePos[i] = 0; // 初始化写位置为0
		pDeviceContext->apUploadBufferReadPos[i] = 0; // 初始化读位置为0
	}
	// DMA缓冲区初始化完成后，设置上传缓冲区索引为0
	pDeviceContext->nUploadBufferWriteIndex = 0;
	pDeviceContext->nUploadBufferReadIndex = 0;

	return TRUE;
}

BOOLEAN HfPciE_DoSetDmaAccess
(
	IN WDFDEVICE hDevice,
	IN EHfPciE_DmaAccess eDmaAccess
)
{
	PDEVICE_CONTEXT pDeviceContext;
	pDeviceContext = DeviceGetContext(hDevice);

	pDeviceContext->eDmaAccess = eDmaAccess;
	return TRUE;
}
