#pragma once

#include <string>
#include <stdexcept>

namespace vx_window_namespace_ {

	class VX_Window_RunTimeError : public std::runtime_error
	{
	public:
		explicit VX_Window_RunTimeError(const std::string &message);
		explicit VX_Window_RunTimeError(const char* message);
		~VX_Window_RunTimeError();
	};
};

