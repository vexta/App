#pragma once

#include "VX_OVR_RunTimeError.h"
#include <VX_Window_Lib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <Extras\OVR_Math.h>

#include <string>
#include <memory>
#include <functional>


namespace vx_ovr_namespace_ {

	class OVRHMDHandle
	{
	public:
		OVRHMDHandle() { };

		// must be called before initialize 
		virtual void setWindowParams(const unsigned short width, const unsigned short height, const std::string &title) = 0;

		virtual void initialize() = 0;
		virtual GLuint prepareFramebuffer(ovrEyeType eye) = 0;
		virtual void submitFrame() = 0;
		virtual void getTrackingState() = 0;

		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, OVR::Vector3f position, OVR::Vector3f front, OVR::Vector3f right, float yaw) const = 0;
		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const = 0;
		virtual OVR::Matrix4f getProjectionMatrix(ovrEyeType eye) const;
		virtual float getUserHeight() = 0;
		virtual void setViewport(ovrEyeType eye) const;
		
		virtual void setKeyCallback(std::function<void(int, int)> keyCallback) = 0;
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback) = 0;

		virtual void setShouldClose(bool shouldClose) = 0;
		virtual bool shouldClose() = 0;

	protected:
		GLuint leftFbo_, rightFbo_;


		OVR::Matrix4f projectionLeft_, projectionRight_;
		ovrSizei texSizeLeft_, texSizeRight_;
		ovrHmdDesc description_;
	};
};
