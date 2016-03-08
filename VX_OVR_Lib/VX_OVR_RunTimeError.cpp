#include "VX_OVR_RunTimeError.h"

vx_ovr_namespace_::VX_OVR_RunTimeError::VX_OVR_RunTimeError(const char * message) :
	std::runtime_error(message)
{
}

vx_ovr_namespace_::VX_OVR_RunTimeError::VX_OVR_RunTimeError(const std::string & message) :
	std::runtime_error(message)
{
}
