#include "OVRHMDHandleWithDevice.h"



vx_ovr_namespace_::OVRHMDHandleWithDevice::OVRHMDHandleWithDevice()
{
}


vx_ovr_namespace_::OVRHMDHandleWithDevice::~OVRHMDHandleWithDevice()
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setWindowParams(const unsigned short width, const unsigned short height, const std::string & title)
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::initialize()
{
	ovrResult result = ovr_Create(&session_, &luid_);
	if (OVR_FAILURE(result)) {
		throw VX_OVR_RunTimeError("Failed to create OVR session");
	}
}

GLuint vx_ovr_namespace_::OVRHMDHandleWithDevice::prepareFramebuffer(ovrEyeType eye)
{
	return GLuint();
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::submitFrame()
{
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleWithDevice::getViewMatrix(ovrEyeType eye, OVR::Vector3f position, OVR::Vector3f front, OVR::Vector3f right, float yaw) const
{
	return OVR::Matrix4f();
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleWithDevice::getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const
{
	return OVR::Matrix4f();
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setKeyCallback(std::function<void(int, int)> keyCallback)
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setMousePosCallback(std::function<void(double, double)> mousePosCallback)
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setShouldClose(bool shouldClose)
{
	if (shouldClose) {
		
	}
}

bool vx_ovr_namespace_::OVRHMDHandleWithDevice::shouldClose()
{
	return false;
}
