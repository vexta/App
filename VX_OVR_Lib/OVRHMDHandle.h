#pragma once

#include <GL/glew.h>

#include <string>
#include <memory>
#include <functional>

#include <OVR_CAPI.h>
#include <Extras\OVR_Math.h>


namespace vx_ovr_namespace_ {

	class OVRHMDHandle
	{
	public:
		OVRHMDHandle() { };
		virtual ~OVRHMDHandle() = 0;

		virtual void initialize() = 0;
		virtual GLuint prepareFramebuffer(ovrEyeType eye) = 0;
		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const = 0;
		virtual OVR::Matrix4f getProjectionMatrix(ovrErrorType eye) const = 0;
		virtual void setKeyCallback(std::function<void(int, int)> keyCallback) = 0;
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback) = 0;

	protected:
		GLuint leftFbo_, rightFbo_;
		ovrHmdDesc description_;
	};
};
