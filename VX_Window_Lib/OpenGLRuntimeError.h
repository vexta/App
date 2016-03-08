#pragma once

#include <stdexcept>
#include <string>

namespace vx_opengl_namespace_ {

	class OpenGLRuntimeError : public std::runtime_error
	{
	public:
		explicit OpenGLRuntimeError(const char* message);
		explicit OpenGLRuntimeError(const std::string &message);
		~OpenGLRuntimeError();
	};
};

