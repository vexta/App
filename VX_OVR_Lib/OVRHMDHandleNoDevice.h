#pragma once

#include <memory>

#include "OVRHMDHandle.h"
#include <VX_Window_Lib.h>

namespace vx_ovr_namespace_ {

	class OVRHMDHandleNoDevice : public OVRHMDHandle
	{
	public:
		OVRHMDHandleNoDevice();
		~OVRHMDHandleNoDevice();

		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const;
		virtual OVR::Matrix4f getProjectionMatrix(ovrErrorType eye) const;
		virtual void setKeyCallback(std::function<void(int, int)> keyCallback);
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback);

	private:
		std::shared_ptr<vxWnd::OpenGLStereoWindow> window_;

	};

};

