/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#define INITGUID
#include "pch.h"

#include <ntddk.h>
#include <wdf.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

extern "C" NDIS_STATUS DriverEntry(_In_ PDRIVER_OBJECT, _In_ PUNICODE_STRING);
EVT_WDF_DRIVER_DEVICE_ADD VirtualAx25EvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP VirtualAx25EvtDriverContextCleanup;

EXTERN_C_END

#define DRIVER_MAJOR_VERSION 0
#define DRIVER_MINOR_VERSION 1