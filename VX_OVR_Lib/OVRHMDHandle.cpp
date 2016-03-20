#include "OVRHMDHandle.h"

OVR::Matrix4f vx_ovr_namespace_::OVRHMDHandle::getProjectionMatrix(ovrEyeType eye) const
{
	return eye == ovrEye_Left ? projectionLeft_ : projectionRight_;
}
