#pragma once

#include "OVRHMDHandle.h"

namespace vx_ovr_namespace_ {
	class OVRHMDHandleWithDevice : public OVRHMDHandle
	{
	public:
		OVRHMDHandleWithDevice();
		virtual ~OVRHMDHandleWithDevice();

		virtual void setWindowParams(const unsigned short width, const unsigned short height, const std::string &title);

		virtual void initialize();
		virtual GLuint prepareFramebuffer(ovrEyeType eye);
		virtual void submitFrame();

		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, OVR::Vector3f position, OVR::Vector3f front, OVR::Vector3f right, float yaw) const;
		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const;
		
		virtual void setKeyCallback(std::function<void(int, int)> keyCallback);
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback);

		virtual void setShouldClose(bool shouldClose);
		virtual bool shouldClose();

	protected:

		ovrSession session_;
		ovrHmdDesc description_;
		ovrGraphicsLuid luid_;
	};
}

