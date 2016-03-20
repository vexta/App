#pragma once

#include "OVRHMDHandle.h"

#include <memory>

namespace vx_ovr_namespace_ {

	class OVRHMDHandleNoDevice : public OVRHMDHandle
	{
	public:
		OVRHMDHandleNoDevice();
		~OVRHMDHandleNoDevice();

		// must be called before initialize 
		virtual void setWindowParams(const unsigned short width, const unsigned short height, const std::string &title);

		virtual void initialize();
		virtual GLuint prepareFramebuffer(ovrEyeType eye);
		virtual void submitFrame();

		virtual OVR::Matrix4f getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const;
		virtual void setViewport() const;
		
		virtual void setKeyCallback(std::function<void(int, int)> keyCallback);
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback);

	protected:
		void initAsDK2();

		GLuint leftTexture_, rightTexture_;
		std::shared_ptr<vxWnd::OpenGLStereoWindow> window_;

	};

};

