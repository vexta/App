#pragma once

#include "GLFWWindow.h"
#include "OpenGLShader.h"

namespace vx_window_namespace_ {

	class GLFWStereoWindow : public GLFWWindow 
	{
	public:
		GLFWStereoWindow();
		GLFWStereoWindow(const unsigned short width, const unsigned short height, const std::string title);
		~GLFWStereoWindow();

		virtual void create();
		virtual void update(GLuint leftTexture, GLuint rightTexture);
		virtual void destroy();
	
	private:
		GLuint quadVAO_;
		vx_opengl_namespace_::OpenGLShader quadShader_;
	};

};
