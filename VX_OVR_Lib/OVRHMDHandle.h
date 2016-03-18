#pragma once

#include "VX_OVR_RunTimeError.h"
#include <VX_Window_Lib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <OVR_CAPI.h>
#include <Extras\OVR_Math.h>

#include <string>
#include <memory>
#include <functional>


namespace vx_ovr_namespace_ {

	class OVRHMDHandle
	{
	public:
		OVRHMDHandle() { };
		virtual ~OVRHMDHandle() = 0;

		// must be called before initialize 
		virtual void setWindowParams(const unsigned short width, const unsigned short height, const std::string &title) = 0;

		virtual void initialize() = 0;
		virtual GLuint prepareFramebuffer(ovrEyeType eye) = 0;
		virtual void submitFrame() = 0;

		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const = 0;
		virtual OVR::Matrix4f getProjectionMatrix(ovrErrorType eye) const = 0;
		virtual void setViewport() const = 0;
		
		virtual void setKeyCallback(std::function<void(int, int)> keyCallback) = 0;
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback) = 0;

	protected:
		GLuint leftFbo_, rightFbo_;
		ovrHmdDesc description_;
	};
};
