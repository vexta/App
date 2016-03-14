#include "GLFWWindow.h"

vx_window_namespace_::GLFWWindow::GLFWWindow()
{
}

vx_window_namespace_::GLFWWindow::GLFWWindow(const unsigned short width, const unsigned short height, const std::string & title) :
	BaseWindow(width, height, title)
{
}


vx_window_namespace_::GLFWWindow::~GLFWWindow()
{
}

void vx_window_namespace_::GLFWWindow::create()
{
	GLFWWrapper::getInstance()->initialize();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	wnd_handle_ = glfwCreateWindow(width_, height_, title_.c_str(), NULL, NULL);

	if (!wnd_handle_) {
		throw VX_Window_RunTimeError("Failed to create GLFW Window");
	}
	*created_ = true;
	
	glfwSetWindowUserPointer(wnd_handle_, this);
	glfwSetKeyCallback(wnd_handle_, staticKeyCallback);
	glfwSetCursorPosCallback(wnd_handle_, staticCursorPosCallback);

	glfwMakeContextCurrent(wnd_handle_);
	auto glew = GLEWWrapper::getInstance();
	if (glew->isInitialized()) {
		glew->initialize();
	}
}

void vx_window_namespace_::GLFWWindow::update()
{
	glfwPollEvents();
	glfwSwapBuffers(wnd_handle_);
}

void vx_window_namespace_::GLFWWindow::destroy()
{
	glfwSetWindowShouldClose(wnd_handle_, 1);
}

bool vx_window_namespace_::GLFWWindow::shouldClose() const
{
	return glfwWindowShouldClose(wnd_handle_);
}

void vx_window_namespace_::GLFWWindow::makeContexCurrent()
{
	if (created_) {
		glfwMakeContextCurrent(wnd_handle_);
	}
}

void vx_window_namespace_::GLFWWindow::selfKeyCallback(int key, int action, int scancode, int mode)
{
	if (keyCallback_) {
		keyCallback_(key, action);
	}
}

void vx_window_namespace_::GLFWWindow::selfCursorPosCallback(double xpos, double ypos)
{
	if (mousePosCallback_) {
		mousePosCallback_(xpos, ypos);
	}
}

void vx_window_namespace_::GLFWWindow::staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window))->selfKeyCallback(key, scancode, action, mode);
}

void vx_window_namespace_::GLFWWindow::staticCursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
	static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window))->selfCursorPosCallback(xpos, ypos);
}
