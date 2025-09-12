#pragma once

#include "DriverDefine.h"
#include "LocalDefine.h"

// 中断描述上下文
typedef struct _INTERRUPT_CONTEXT
{
    BOOLEAN bInterruptEnable;
    LONG bDmaInterruptHandlerA;
    LONG bDmaInterruptHandlerB;
} INTERRUPT_CONTEXT, *PINTERRUPT_CONTEXT;

typedef struct _INTERRUPT_STATUS
{
    // DMA中断状态标志
    unsigned long DmaInterruptStatusA;
    unsigned long DmaInterruptStatusB;
} INTERRUPT_STATUS, *PINTERRUPT_STATUS;
//
// This macro will generate an inline function called InterruptGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(INTERRUPT_CONTEXT, InterruptGetContext)

NTSTATUS HfPciE_InitializeIsrDpc(_In_ WDFDEVICE hDevice, _Inout_ PDEVICE_CONTEXT pDeviceContext);

BOOLEAN HfPciE_AddUploadBuffer(IN PDEVICE_CONTEXT pDeviceContext,  // 设备对象句柄
                               IN PVOID pUploadBufferAddress,      // 上传缓冲区地址
                               IN ULONG nUploadBufferLength        // 上传缓冲区长度
);

BOOLEAN HfPciE_GetUploadBuffer(IN PDEVICE_CONTEXT pDeviceContext, IN void* pReadBuffer,
                               _Inout_ PULONG nReadBufferLength);

typedef struct _ISR_FIELDS
{
    UCHAR FPGA2PC;  // bit 0  调试用
    UCHAR BufferA;  // bit 1  中断类型BufferA
    UCHAR BufferB;  // bit 2  中断类型BufferB
    UCHAR PC2FPGA;  // bit 3  调试用
    ULONG Length;   // bit 31:7 上传长度

} ISR_FIELDS;

static __forceinline ISR_FIELDS HfPciE_DecodeISR(ULONG raw)
{
    ISR_FIELDS f = {0, 0, 0, 0, 0};
    f.FPGA2PC = (raw >> 0) & 0x1u;
    f.BufferA = (raw >> 1) & 0x1u;
    f.BufferB = (raw >> 2) & 0x1u;
    f.PC2FPGA = (raw >> 3) & 0x1u;
    f.Length = ((raw >> 7) & 0x1FFFFFFu) << 2;  // ->Byte
    return f;
}

EVT_WDF_INTERRUPT_ISR HfPciE_EvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC HfPciE_EvtInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE HfPciE_EvtInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE HfPciE_EvtInterruptDisable;