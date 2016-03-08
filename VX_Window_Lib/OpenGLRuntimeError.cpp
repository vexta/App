#include "OpenGLRuntimeError.h"





vx_opengl_namespace_::OpenGLRuntimeError::OpenGLRuntimeError(const char * message) :
	std::runtime_error(message)
{
}

vx_opengl_namespace_::OpenGLRuntimeError::OpenGLRuntimeError(const std::string & message) :
	std::runtime_error(message)
{
}

vx_opengl_namespace_::OpenGLRuntimeError::~OpenGLRuntimeError()
{
}
