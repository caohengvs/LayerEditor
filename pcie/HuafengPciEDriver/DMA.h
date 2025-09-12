#pragma once


typedef struct _DMA_TRANSACTION_CONTEXT
{
	// Request对象
	WDFREQUEST hRequest;

} DMA_TRANSACTION_CONTEXT, * PDMA_TRANSACTION_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DMA_TRANSACTION_CONTEXT, DmaTransactionGetContext)


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
);
