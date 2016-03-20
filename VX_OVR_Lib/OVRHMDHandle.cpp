#include "OVRHMDHandle.h"

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandle::getProjectionMatrix(ovrEyeType eye) const
{
	return eye == ovrEye_Left ? projectionLeft_ : projectionRight_;
}

void vx_ovr_namespace_::OVRHMDHandle::setViewport(ovrEyeType eye) const
{
	if (eye == ovrEye_Left) {
		glViewport(0, 0, texSizeLeft_.w, texSizeRight_.h);
	}
	else {
		glViewport(0, 0, texSizeRight_.w, texSizeRight_.h);
	}
}
