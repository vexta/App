#include "OVRWrapper.h"

vx_ovr_namespace_::OVRWrapper::~OVRWrapper()
{
}

void vx_ovr_namespace_::OVRWrapper::initialize()
{
	if (!initialized_) {
		ovrResult result = ovr_Initialize(NULL);

		if (OVR_FAILURE(result)) {
			// failed to initialize Oculus SDK, use OVRHMDHandle
			useRealHMD_ = false;
		}
		else {
			// OVRLib is initialized
			useRealHMD_ = true;

		}
		initialized_ = true;
	}
}

bool vx_ovr_namespace_::OVRWrapper::isInitialized()
{
	return initialized_;
}

void vx_ovr_namespace_::OVRWrapper::shutDown()
{
	ovr_Shutdown();
}

std::shared_ptr<vx_ovr_namespace_::OVRHMDHandle> vx_ovr_namespace_::OVRWrapper::getOVRHMDDeviceHandle()
{
	initialize();
	if (useRealHMD_) {
		return std::shared_ptr<OVRHMDHandleWithDevice>(new OVRHMDHandleWithDevice());
	}
	else {
		return std::shared_ptr<OVRHMDHandleNoDevice>(new OVRHMDHandleNoDevice());
	}
}

std::shared_ptr<vx_ovr_namespace_::OVRWrapper> vx_ovr_namespace_::OVRWrapper::getInstance()
{
	static std::shared_ptr<OVRWrapper> instance(new OVRWrapper());
	return instance;
}

vx_ovr_namespace_::OVRWrapper::OVRWrapper() :
	initialized_(false), useRealHMD_(false)
{
}
