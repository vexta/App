#include "OVRHMDHandleWithDevice.h"



vx_ovr_namespace_::OVRHMDHandleWithDevice::OVRHMDHandleWithDevice()
{
	// create session
	ovrResult result = ovr_Create(&session_, &luid_);
	if (OVR_FAILURE(result) || session_ == nullptr) {
		throw VX_OVR_RunTimeError("Failed to create OVR session");
		return;
	}

	description_ = ovr_GetHmdDesc(session_);
}


vx_ovr_namespace_::OVRHMDHandleWithDevice::~OVRHMDHandleWithDevice()
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setWindowParams(const unsigned short width, const unsigned short height, const std::string & title)
{
	window_ = std::make_shared<vxWnd::OpenGLWindow>(width, height, title);
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::initialize()
{

	// create window
	window_->create();
	window_->makeContexCurrent();


	// configure head-tracking
	configureTracking();

	// create swap texture set
	createTextureSet();

}

GLuint vx_ovr_namespace_::OVRHMDHandleWithDevice::prepareFramebuffer(ovrEyeType eye)
{
	if (eye == ovrEye_Right) {
		textureSetRight_->CurrentIndex = (textureSetRight_->CurrentIndex + 1) % textureSetRight_->TextureCount;
		auto tex = reinterpret_cast<ovrGLTexture*>(&textureSetRight_->Textures[textureSetRight_->CurrentIndex]);
		textureRight_ = tex->OGL.TexId;
		glBindFramebuffer(GL_FRAMEBUFFER, fboRight_);
		glViewport(0, 0, texSizeRight_.w, texSizeRight_.h);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);
		return fboRight_;
	}
	else {
		textureSetLeft_->CurrentIndex = (textureSetLeft_->CurrentIndex + 1) % textureSetLeft_->TextureCount;
		auto tex = reinterpret_cast<ovrGLTexture*>(&textureSetLeft_->Textures[textureSetLeft_->CurrentIndex]);
		textureLeft_ = tex->OGL.TexId;
		glBindFramebuffer(GL_FRAMEBUFFER, fboLeft_);
		glViewport(0, 0, texSizeLeft_.w, texSizeLeft_.h);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
		glEnable(GL_FRAMEBUFFER_SRGB);
		return fboLeft_;
	}
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::submitFrame()
{
	window_->update();
	ovrViewScaleDesc viewScaleDesc;
	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
	viewScaleDesc.HmdToEyeViewOffset[0] = eyeRenderDesc_[0].HmdToEyeViewOffset;
	viewScaleDesc.HmdToEyeViewOffset[1] = eyeRenderDesc_[1].HmdToEyeViewOffset;

	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

	ld.ColorTexture[0] = textureSetLeft_;
	ld.Viewport[0] = OVR::Recti(texSizeLeft_);
	ld.Fov[0] = description_.DefaultEyeFov[0];
	ld.RenderPose[0] = eyeRenderPosef_[0];

	ld.ColorTexture[1] = textureSetRight_;
	ld.Viewport[1] = OVR::Recti(texSizeRight_);
	ld.Fov[1] = description_.DefaultEyeFov[1];
	ld.RenderPose[1] = eyeRenderPosef_[1];

	ld.SensorSampleTime = sampleTime_;

	ovrLayerHeader* layers = &ld.Header;
	ovr_SubmitFrame(session_, 0, &viewScaleDesc, &layers, 1);
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::getTrackingState()
{
	ovrTrackingState ts = ovr_GetTrackingState(session_, ovr_GetTimeInSeconds(), false);
	ovr_CalcEyePoses(ts.HeadPose.ThePose, viewOffset_, eyeRenderPosef_);
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleWithDevice::getViewMatrix(ovrEyeType eye, OVR::Vector3f position, OVR::Vector3f front, OVR::Vector3f right, float yaw) const
{
	return getViewMatrix(eye, position.x, position.y, position.z, yaw);
}

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandleWithDevice::getViewMatrix(ovrEyeType eye, float pos_x, float pos_y, float pos_z, float yaw) const
{
	OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(yaw);
	OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f(eyeRenderPosef_[eye].Orientation);
	OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0.0, 1.0, 0.0));
	OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0.0, 0.0, -1.0));
	OVR::Vector3f shiftedEyePos = OVR::Vector3f(pos_x, pos_y, pos_z) + rollPitchYaw.Transform(eyeRenderPosef_[eye].Position);

	return OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setKeyCallback(std::function<void(int, int)> keyCallback)
{
	window_->setKeyCallback(keyCallback);
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setMousePosCallback(std::function<void(double, double)> mousePosCallback)
{
	window_->setMousePosCallback(mousePosCallback);
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::setShouldClose(bool shouldClose)
{
	if (shouldClose) {
		window_->destroy();
	}
}

bool vx_ovr_namespace_::OVRHMDHandleWithDevice::shouldClose()
{
	return false;
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::configureTracking()
{
	sampleTime_ = ovr_GetTimeInSeconds();
	ovrResult result = ovr_ConfigureTracking(session_, description_.AvailableTrackingCaps, description_.AvailableTrackingCaps);

	if (OVR_FAILURE(result)) {
		ovr_Shutdown();
		throw new VX_OVR_RunTimeError("Failed to configure OVR tracking");
	}

	eyeRenderDesc_[0] = ovr_GetRenderDesc(session_, ovrEye_Left, description_.DefaultEyeFov[0]);
	eyeRenderDesc_[1] = ovr_GetRenderDesc(session_, ovrEye_Right, description_.DefaultEyeFov[1]);
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::createSession()
{
}

void vx_ovr_namespace_::OVRHMDHandleWithDevice::createTextureSet()
{
	int i;
	ovrResult result;

	texSizeLeft_ = ovr_GetFovTextureSize(session_, ovrEye_Left, description_.DefaultEyeFov[0], 1);
	result = ovr_CreateSwapTextureSetGL(session_, GL_SRGB8_ALPHA8, texSizeLeft_.w, texSizeLeft_.h, &textureSetLeft_);
	if (!OVR_SUCCESS(result)) {
		throw new VX_OVR_RunTimeError("Failed to create Texture Set for Left Eye");
	}

	for (i = 0; i < textureSetLeft_->TextureCount; ++i) {
		ovrGLTexture *tex = (ovrGLTexture *)&textureSetLeft_->Textures[i];
		glBindTexture(GL_TEXTURE_2D, tex->OGL.TexId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	fboLeft_ = vxWnd::GLEWWrapper::generateFramebufferObject(texSizeLeft_.w, texSizeLeft_.h);

	texSizeRight_ = ovr_GetFovTextureSize(session_, ovrEye_Right, description_.DefaultEyeFov[0], 1);
	result = ovr_CreateSwapTextureSetGL(session_, GL_SRGB8_ALPHA8, texSizeRight_.w, texSizeRight_.h, &textureSetRight_);

	if (!OVR_SUCCESS(result)) {
		throw new VX_OVR_RunTimeError("Failed to create Texture Set for Right Eye");
	}

	for (i = 0; i < textureSetRight_->TextureCount; ++i) {
		ovrGLTexture *tex = (ovrGLTexture *)&textureSetLeft_->Textures[i];
		glBindTexture(GL_TEXTURE_2D, tex->OGL.TexId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	fboRight_ = vxWnd::GLEWWrapper::generateFramebufferObject(texSizeRight_.w, texSizeRight_.h);

	glBindTexture(GL_TEXTURE_2D, 0);
}

