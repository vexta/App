#include "OVRHMDHandleWithDevice.h"



vx_ovr_namespace_::OVRHMDHandleWithDevice::OVRHMDHandleWithDevice()
{
}


vx_ovr_namespace_::OVRHMDHandleWithDevice::~OVRHMDHandleWithDevice()
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::initialize()
{
	ovrResult result = ovr_Create(&session_, &luid_);
	if (OVR_FAILURE(result)) {
		throw VX_OVR_RunTimeError("Failed to create OVR session");
	}
}
