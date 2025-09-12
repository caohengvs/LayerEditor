#pragma once

#include "DriverDefine.h"
#include "LocalDefine.h"

// Ö´ÐÐÈí¼þ¸´Î»
BOOLEAN HfPciE_DoSoftwareReset
(
	IN WDFDEVICE hDevice
);

BOOLEAN HfPciE_DoSetDmaAccess
(
	IN WDFDEVICE hDevice,
	IN EHfPciE_DmaAccess eDmaAccess
);
