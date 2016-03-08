#pragma once

#include "BaseWindow.h"
#include "VX_Window_RunTimeError.h"
#include "GLEWWrapper.h"
#include "GLFWWrapper.h"

namespace vx_window_namespace_ {

	class GLFWWindow : public BaseWindow
	{
	public:
		GLFWWindow();
		GLFWWindow(const unsigned short width, const unsigned short height, const std::string &title);
		~GLFWWindow();

		// BaseWindow related functions
		virtual void create();
		virtual void update();
		virtual void destroy();
		
		virtual bool shouldClose() const;

		// OpenGL related functions

		// GLFW related functions
		virtual void makeContexCurrent();

	protected:
		GLFWwindow *wnd_handle_;

		// callback handlers
		void selfKeyCallback(int, int, int, int);
		void selfCursorPosCallback(double, double);

		// real callbacks from GLFW library, GLFW can't call this.callback
		static void staticKeyCallback(GLFWwindow*, int, int, int, int);
		static void staticCursorPosCallback(GLFWwindow*, double, double);
	};

}