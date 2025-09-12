/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include "public.h"
#include "DriverDefine.h"
#include "LocalDefine.h"
EXTERN_C_START

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    //////////////////////////////////////////////////////////////////////////
    // 状态相关
    // Pnp状态
	EHfDevicePnPState eDevicePnPState;
	EHfDevicePnPState ePreviousPnPState;

    // Power State，标识进入D0状态
    BOOLEAN bPowerD0State;

    // 打开状态
    BOOLEAN bFileOpenState;

    // 状态相关 End
    //////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// 资源相关
	// 转换后的MemoryBar基地址数组
	PVOID apMemBaseAddress[HF_MAX_SUPPORT_MEMORY_BAR_NUMBER];
    // 对应MemoryBar最大访问长度
	ULONG anMemLength[HF_MAX_SUPPORT_MEMORY_BAR_NUMBER];
    // 有效MemoryBar数量
    ULONG nMemoryBarNumber;

    // 资源相关 End
    //////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IO操作相关
	// IO队列句柄
	WDFQUEUE				hIoQueue;
	// IO操作的自旋锁
	WDFSPINLOCK				hIoSpinLock;

    // IO操作相关 End
    //////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// 中断相关
	// 中断句柄
	CM_PARTIAL_RESOURCE_DESCRIPTOR oInterruptDescRaw;
	CM_PARTIAL_RESOURCE_DESCRIPTOR oInterruptDescTranslated;
	WDFINTERRUPT			hInterrupt;

    // 中断相关 End
    //////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// DMA相关
    EHfPciE_DmaAccess eDmaAccess;
	// DmaEnabler
	WDFDMAENABLER hDmaEnabler;

	// Common buffer(数据上传和DDR读写共用)
	WDFCOMMONBUFFER hDmaCommBufA;
	WDFCOMMONBUFFER hDmaCommBufB;
	PVOID pDmaCommBufAddressA;
	PVOID pDmaCommBufAddressB;
	PHYSICAL_ADDRESS oDmaCommBufPhyAddressA;
	PHYSICAL_ADDRESS oDmaCommBufPhyAddressB;
	ULONG nDmaCommonBufferLength;
	KSPIN_LOCK spinLockCommBufferA;
	KSPIN_LOCK spinLockCommBufferB;

	//Common buffer(数据上传和DDR读写共用)
	WDFCOMMONBUFFER hUploadBuffer;// 用于数据上传的DMA缓冲区
	PVOID pUploadBufferAddress;// 数据上传的DMA缓冲区地址
	PHYSICAL_ADDRESS oUploadBufferPhyAddress;// 数据上传的DMA缓冲区物理地址
	ULONG nUploadBufferLength;// 数据上传的DMA缓冲区长度
	PVOID apUploadBuffer[HF_DMA_UPLOAD_BUFFER_COUNT];// 用于数据上传的DMA缓冲区地址数组
	ULONG apUploadBufferStatus[HF_DMA_UPLOAD_BUFFER_COUNT];// 用于数据上传的DMA缓冲区状态数组
	ULONG apUploadBufferWritePos[HF_DMA_UPLOAD_BUFFER_COUNT];// 用于数据上传的DMA缓冲区写位置数组
	ULONG apUploadBufferReadPos[HF_DMA_UPLOAD_BUFFER_COUNT];// 用于数据上传的DMA缓冲区读位置数组
	ULONG nUploadBufferWriteIndex; // 数据上传缓冲区写索引
	ULONG nUploadBufferReadIndex; // 数据上传缓冲区读索引

	WDFCOMMONBUFFER hDmaIsrBuf;// 中断处理使用的DMA缓冲区
	PVOID pDmaIsrBufAddress;//中断处理使用的DMA缓冲区地址
	PHYSICAL_ADDRESS oDmaIsrBufPhyAddress;//中断处理使用的DMA缓冲区物理地址
	ULONG nDmaIsrBufferLength;// 中断处理使用的DMA缓冲区长度
	//////////////////////////////////////////////////////////////////////////
    // DMA相关监控线程信息
    //////////////////////////////////////////////////////////////////////////
	/*PETHREAD hThread;             // 线程句柄
	KEVENT oThreadEvent;           // 事件对象
	KTIMER oThreadTimer;           // 计时器对象
	LARGE_INTEGER oThreadDelay;    // 延迟时间
	BOOLEAN bExitThread;          // 退出标志
	*/
	//////////////////////////////////////////////////////////////////////////
	// DMA相关监控线程信息结束
	//////////////////////////////////////////////////////////////////////////
	WDFINTERRUPT hInterrupts[MAX_MSI_INTERRUPTS]; // 多中断对象数组
	ULONG InterruptCount;                        // 实际注册的中断数
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;


//
// The struct performs as configure to fpga used in IsrDpc
//
typedef unsigned long DWORD;
struct fpgaDescriptorConfigure
{
	int nBarID;
	DWORD address[6];
	DWORD dwValue[6];
};
struct fpgaDescriptorConfigure fpga_DesConf;

struct fpgaFreezeConfigure
{
	int nBarID;
	DWORD address;
	DWORD dwValue;
};
struct fpgaFreezeConfigure fpga_FreezeConf;

struct fpgaReadyConfigure
{
	int nBarID;
	DWORD address;
	DWORD dwValue;
};
struct fpgaReadyConfigure fpga_ReadyConf;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
HfPciE_CreateDevice(
    _Inout_ PWDFDEVICE_INIT pDeviceInit
    );

// 初始化设备上下文
NTSTATUS HfPciE_InitDeviceContext
(
	IN WDFDEVICE hDevice,
	_Inout_ PDEVICE_CONTEXT pDeviceContext
);


EXTERN_C_END
