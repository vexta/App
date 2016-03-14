#include "GLFWWrapper.h"


std::shared_ptr<vx_window_namespace_::GLFWWrapper> vx_window_namespace_::GLFWWrapper::getInstance()
{
	static std::shared_ptr<GLFWWrapper> instance(new GLFWWrapper());
	return instance;
}

vx_window_namespace_::GLFWWrapper::GLFWWrapper() :
	initialized_(false)
{
}


vx_window_namespace_::GLFWWrapper::~GLFWWrapper()
{
	glfwTerminate();
}

void vx_window_namespace_::GLFWWrapper::initialize()
{
	if (!initialized_) {
		if (!glfwInit()) {
			throw VX_Window_RunTimeError("GLFW Library initialization fail");
		}
		initialized_ = true;
	}
}

bool vx_window_namespace_::GLFWWrapper::isInitialized() const
{
	return initialized_;
}


