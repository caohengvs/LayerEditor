/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

EXTERN_C_START

#include "DriverDefine.h"
#include "LocalDefine.h"
//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

	// 写MemeryBar参数
    HF_PCIE_RW_PARAM oWriteMemeryBar;

	// 写DDR内存
    HF_PCIE_RW_PARAM oWriteDDR;

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
HfPciE_QueueInitialize(
    _In_ WDFDEVICE Device
    );

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL HfPciE_EvtIoDeviceControl;
//EVT_WDF_IO_QUEUE_IO_STOP HfPciE_EvtIoStop;

EVT_WDF_IO_QUEUE_IO_READ HfPciE_EvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE HfPciE_EvtIoWrite;

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
);

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
);

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
);

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
);



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
);

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
);

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
);

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
);


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
);


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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);


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
);


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
);

EXTERN_C_END
