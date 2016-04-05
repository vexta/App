#include "OVRHMDHandleNoDevice.h"

#include <AntTweakBar.h>

vx_ovr_namespace_::OVRHMDHandleNoDevice::OVRHMDHandleNoDevice()
{
}


vx_ovr_namespace_::OVRHMDHandleNoDevice::~OVRHMDHandleNoDevice()
{
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::setWindowParams(const unsigned short width, const unsigned short height, const std::string & title)
{
	window_ = std::make_shared<vxWnd::OpenGLStereoWindow>(width, height, title);
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::initialize()
{
	window_->create();

	window_->makeContexCurrent();
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_->getWidth(), window_->getHeight());
	TwBar *quaternionGUI = TwNewBar("Head Orientation");
	//TwSetParam(quaternionGUI, NULL, "position", TW_PARAM_CSTRING, 1, "-200 -200");
	
	TwAddVarRW(quaternionGUI, "Quaternion", TW_TYPE_QUAT4F, &eye_orientation_, "showval=true opened=true ");

	auto mousePosCallback = [](double x, double y) {
		if (TwEventMousePosGLFW(x, y)) {
			return;
		}
	};

	auto mouseButtonCallback = [](int button, int action, int mods) {
		if (TwEventMouseButtonGLFW(button, action)) {
			return;
		}
	};


	window_->setMousePosCallback(mousePosCallback);
	window_->setMouseButtonCallback(mouseButtonCallback);

	// initialize properties
	initAsDK2();

	// initialize framebuffers
	std::pair<GLuint, GLuint> fbo = vxWnd::GLEWWrapper::generateFramebufferObjectWithTexture(texSizeLeft_.w, texSizeRight_.h);
	leftFbo_ = fbo.first;
	leftTexture_ = fbo.second;

	fbo = vxWnd::GLEWWrapper::generateFramebufferObjectWithTexture(texSizeRight_.w, texSizeRight_.h);
	rightFbo_ = fbo.first;
	rightTexture_ = fbo.second;
}

GLuint vx_ovr_namespace_::OVRHMDHandleNoDevice::prepareFramebuffer(ovrEyeType eye)
{
	return eye == ovrEye_Left ? leftFbo_ : rightFbo_;
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::submitFrame()
{
	window_->update(leftTexture_, rightTexture_);
	window_->makeContexCurrent();
	glViewport(0, 0, window_->getWidth(), window_->getHeight());
	TwDraw();
	glfwSwapBuffers(window_->getHandle());
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleNoDevice::getViewMatrix(ovrEyeType eye, OVR::Vector3f position, OVR::Vector3f front, OVR::Vector3f right, float yaw) const
{
	OVR::Vector3f shiftedEyePos;

	OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(yaw);
	OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f(eye_orientation_); // multiply with user head orientation matrix
	OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
	OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
	if (eye == ovrEye_Left) {
		shiftedEyePos = position + finalRollPitchYaw.Transform((-right * 0.1f) + OVR::Vector3f(0, 1.8f, 0));
	}
	else {
		shiftedEyePos = position + finalRollPitchYaw.Transform((right * 0.1f) + OVR::Vector3f(0, 1.8f, 0));
	}

	return OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleNoDevice::getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const
{
	ovrPosef *eyeRenderPosef;
	float user_height_;

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

void vx_ovr_namespace_::OVRHMDHandleNoDevice::setKeyCallback(std::function<void(int, int)> keyCallback)
{
	window_->setKeyCallback(keyCallback);
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::setMousePosCallback(std::function<void(double, double)> mousePosCallback)
{
	window_->setMousePosCallback(mousePosCallback);
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::setShouldClose(bool shouldClose)
{
	if (shouldClose) {
		window_->destroy();
	}
}

bool vx_ovr_namespace_::OVRHMDHandleNoDevice::shouldClose()
{
	return window_->shouldClose();
}

void vx_ovr_namespace_::OVRHMDHandleNoDevice::initAsDK2()
{
	texSizeLeft_.w = 1182;
	texSizeLeft_.h = 1464;
	texSizeRight_.w = 1182;
	texSizeRight_.h = 1464;

	// from watches when debugging
	description_.DefaultEyeFov[ovrEye_Left].UpTan = 1.33160317f;
	description_.DefaultEyeFov[ovrEye_Left].DownTan = 1.33160317f;
	description_.DefaultEyeFov[ovrEye_Left].LeftTan = 1.05865765f;
	description_.DefaultEyeFov[ovrEye_Left].RightTan = 1.09236801f;

	description_.DefaultEyeFov[ovrEye_Right].UpTan = 1.33160317f;
	description_.DefaultEyeFov[ovrEye_Right].DownTan = 1.33160317f;
	description_.DefaultEyeFov[ovrEye_Right].LeftTan = 1.09236801f;
	description_.DefaultEyeFov[ovrEye_Right].RightTan = 1.05865765f;

	// init projection matrix
	projectionLeft_ = ovrMatrix4f_Projection(description_.DefaultEyeFov[ovrEye_Left], 0.2f, 1000.0f, ovrProjection_RightHanded);
	projectionRight_ = ovrMatrix4f_Projection(description_.DefaultEyeFov[ovrEye_Right], 0.2f, 1000.0f, ovrProjection_RightHanded);
}
