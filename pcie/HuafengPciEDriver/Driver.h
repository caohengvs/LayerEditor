/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <wdm.h>        // WDM 驱动模型头文件
#include <wdfdevice.h >

#include "device.h"
#include "queue.h"
#include "trace.h"
#include "PowerPnp.h"
#include "FileObject.h"
#include "DMA.h"
#include "IsrDpc.h"
#include "LocalFunction.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD HfPciE_EvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP HfPciE_EvtDriverContextCleanup;

EXTERN_C_END
