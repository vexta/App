#include "OVRWrapper.h"

vx_ovr_namespace_::OVRWrapper::~OVRWrapper()
{
}

void vx_ovr_namespace_::OVRWrapper::initialize()
{
	if (!initialized_) {
		
	}
}

bool vx_ovr_namespace_::OVRWrapper::isInitialized()
{
	return initialized_;
}

std::shared_ptr<vx_ovr_namespace_::OVRWrapper> vx_ovr_namespace_::OVRWrapper::getInstance()
{
	static std::shared_ptr<OVRWrapper> instance(new OVRWrapper());
	return instance;
}
