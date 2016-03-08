#pragma once

#include <stdexcept>
#include <string>

namespace vx_ovr_namespace_ {

	class VX_OVR_RunTimeError : public std::runtime_error
	{
	public:
		explicit VX_OVR_RunTimeError(const char *message);
		explicit VX_OVR_RunTimeError(const std::string &message);
	};
};

