#include "GLEWWrapper.h"



vx_window_namespace_::GLEWWrapper::GLEWWrapper()
{
}


vx_window_namespace_::GLEWWrapper::~GLEWWrapper()
{
}

void vx_window_namespace_::GLEWWrapper::initialize()
{
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		throw VX_Window_RunTimeError("GLEW initialization failed");
	}
	initialized_ = true;
}

bool vx_window_namespace_::GLEWWrapper::isInitialized() const
{
	return initialized_;
}

std::shared_ptr<vx_window_namespace_::GLEWWrapper> vx_window_namespace_::GLEWWrapper::getInstance()
{
	static std::shared_ptr<GLEWWrapper> instance(new GLEWWrapper);
	return instance;
}
