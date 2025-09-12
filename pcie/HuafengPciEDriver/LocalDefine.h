#pragma once
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
//////////////////////////////////////////////////////////////////////////
// 定义宏
// 定义最多支持的MemoryBar的数量
#define HF_MAX_SUPPORT_MEMORY_BAR_NUMBER		32
// 定义最多支持的DDR BAR的数量
#define HF_MAX_SUPPORT_DDR_BAR_NUMBER			1

// 标记I/O请求时，不限制请求的长度
#define HF_PCIE_IO_DEFAULT_REQ_LENGTH		0

// 
#define HW_PCI_E_INITIALIZE_PNP_STATE(_DeviceContext_)    \
	(_DeviceContext_)->eDevicePnPState = ePNPState_NotStarted; \
	(_DeviceContext_)->ePreviousPnPState = ePNPState_NotStarted; 

#define HW_PCI_E_SET_NEW_PNP_STATE(_DeviceContext_, _NewState_) \
	(_DeviceContext_)->ePreviousPnPState = (_DeviceContext_)->eDevicePnPState; \
	(_DeviceContext_)->eDevicePnPState = (_NewState_); \
	KdPrint(("HuafengPciEDriver.sys: Set state to %s from %s\n", apStateNames[(_DeviceContext_)->eDevicePnPState], apStateNames[(_DeviceContext_)->ePreviousPnPState]));

#define HW_PCI_E_RESTORE_PREVIOUS_PNP_STATE(_DeviceContext_)   \
	KdPrint(("HuafengPciEDriver.sys: Restore state to %s from %s\n", apStateNames[(_DeviceContext_)->ePreviousPnPState], apStateNames[(_DeviceContext_)->eDevicePnPState])); \
	(_DeviceContext_)->eDevicePnPState = (_DeviceContext_)->ePreviousPnPState; 



// 定义读/写MemeryBar的宏
// 读取寄存器（DWORD）
#define BfPciE_ReadRegister( /*IN PVOID*/pBaseAddress, /*IN ULONG*/nBaseAddressOffset, /*OUT PULONG*/pValue)	\
	*pValue = READ_REGISTER_ULONG((PULONG)((PUCHAR)(pBaseAddress) + (nBaseAddressOffset)))

// 读取MemeryBar
#define BfPciE_ReadMemeryBar( /*IN PVOID*/pBaseAddress, /*IN ULONG*/nBaseAddressOffset, /*OUT PULONG*/pBuffer, /*IN ULONG*/nCount)	\
	READ_REGISTER_BUFFER_ULONG((PULONG)((PUCHAR)(pBaseAddress) + (nBaseAddressOffset)), (pBuffer), (nCount))

//	写入寄存器（DWORD）
#define BfPciE_WriteRegister( /*IN PVOID*/pBaseAddress, /*IN ULONG*/nBaseAddressOffset, /*IN ULONG*/nValue)	\
		WRITE_REGISTER_ULONG((PULONG)((PUCHAR)(pBaseAddress) + (nBaseAddressOffset)), (nValue))

// 写入MemeryBar
#define BfPciE_WriteMemeryBar( /*IN PVOID*/pBaseAddress, /*IN ULONG*/nBaseAddressOffset, /*IN PULONG*/pValueBuf, /*IN ULONG*/nCount)	\
	WRITE_REGISTER_BUFFER_ULONG((PULONG)((PUCHAR)(pBaseAddress) + (nBaseAddressOffset)), (pValueBuf), (nCount))


//////////////////////////////////////////////////////////////////////////
// 定义枚举

// 定义PNP的状态
typedef enum _ENUM_HF_PCI_E_DEVICE_PNP_STATE
{
	ePNPState_NotStarted,				// 设置还没有启动
	ePNPState_Deleted,					// 设备已经收到过REMOVE_DEVICE IRP
	ePNPState_Started					// 设备已经收到过START_DEVICE IRP

}EHfDevicePnPState;


//////////////////////////////////////////////////////////////////////////
// 定义结构体


// DMA
