/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_VirtualAx25,
    0xcb87cb36,0x8c86,0x4932,0x84,0x11,0xca,0x6a,0x08,0x62,0xe1,0x06);
// {cb87cb36-8c86-4932-8411-ca6a0862e106}
