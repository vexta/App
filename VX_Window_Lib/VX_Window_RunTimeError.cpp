#include "VX_Window_RunTimeError.h"


vx_window_namespace_::VX_Window_RunTimeError::VX_Window_RunTimeError(const std::string &message) :
	runtime_error(message)
{
}

vx_window_namespace_::VX_Window_RunTimeError::VX_Window_RunTimeError(const char *message) :
	runtime_error(message)
{
}

vx_window_namespace_::VX_Window_RunTimeError::~VX_Window_RunTimeError()
{
}
