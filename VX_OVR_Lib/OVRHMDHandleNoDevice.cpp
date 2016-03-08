#include "OVRHMDHandleNoDevice.h"



vx_ovr_namespace_::OVRHMDHandleNoDevice::OVRHMDHandleNoDevice()
{
}


vx_ovr_namespace_::OVRHMDHandleNoDevice::~OVRHMDHandleNoDevice()
{
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleNoDevice::getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const
{
	OVR::Vector3f position(pos_x, pos_y, pos_z);
	position.y += user_height_;

	OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(yaw);
	// TODO replace eyeRenderPosef[eye].Orientation with quaternion from AntTweakBar-thing
	OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f(eyeRenderPosef[eye].Orientation);
	OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
	OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
	OVR::Vector3f shiftedEyePos = position + rollPitchYaw.Transform(eyeRenderPosef[eye].Position);
	return OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleNoDevice::getProjectionMatrix(ovrErrorType eye) const
{
	// TODO projection near and far plane?
	return ovrMatrix4f_Projection(description_.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_RightHanded);
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::setKeyCallback(std::function<void(int, int)> keyCallback)
{
	window_->setKeyCallback(keyCallback);
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::setMousePosCallback(std::function<void(double, double)> mousePosCallback)
{
	window_->setMousePosCallback(mousePosCallback);
}
