#pragma once
#include "DriverDefine.h"
#include "LocalDefine.h"


// ×¢²áPowerPnp
void HfPciE_RegisterPowerPnpEvt
(
	_Inout_ PWDFDEVICE_INIT pDeviceInit
);


EVT_WDF_DEVICE_D0_ENTRY HfPciE_EvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT HfPciE_EvtDeviceD0Exit;

EVT_WDF_DEVICE_PREPARE_HARDWARE HfPciE_EvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE HfPciE_EvtDeviceReleaseHardware;