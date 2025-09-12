#pragma once
#include "DriverDefine.h"
#include "LocalDefine.h"

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _FILE_OBJECT_CONTEXT
{
	BOOLEAN bOpenState;

} FILE_OBJECT_CONTEXT, * PFILE_OBJECT_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_OBJECT_CONTEXT, FileObjectGetContext)

// 注册文件对象
void HfPciE_RegisterFileObjectEvt
(
	_Inout_ PWDFDEVICE_INIT pDeviceInit
);


EVT_WDF_DEVICE_FILE_CREATE HfPciE_EvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE HfPciE_EvtFileClose;