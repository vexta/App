#pragma once

#include "BaseWindow.h"
#include "VX_Window_RunTimeError.h"

namespace vxWnd = vx_window_namespace_;

#define VX_GLEW_GLFW
#ifdef  VX_GLEW_GLFW

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "GLEWWrapper.h"
#include "OpenGLRuntimeError.h"
#include "GLFWWrapper.h"
#include "GLFWWindow.h"
#include "GLFWStereoWindow.h"

namespace vx_window_namespace_ {
	typedef GLFWWindow OpenGLWindow;
	typedef GLFWStereoWindow OpenGLStereoWindow;
};

#endif