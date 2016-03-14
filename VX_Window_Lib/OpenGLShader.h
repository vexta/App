#pragma once

#include "GLEWWrapper.h"
#include "VX_Window_RunTimeError.h"
#include "OpenGLRuntimeError.h"

#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <utility>

namespace vx_opengl_namespace_ {

	class OpenGLShader
	{
	public:
		OpenGLShader();
		~OpenGLShader();

		void create();
		void attachShader(const std::string &code, GLenum type);
		void attachShaderFile(const std::string &path, GLenum type);
		void compileAndLink();
		void deleteShader();

		/* FUTURE PLANS
		void collectAllUniforms();
		void bind();
		void setUnifromValue();
		void 
		*/
		
		operator GLuint();
	private:
		std::string vertexCode_, fragmentCode_;
		
		bool created_;
		bool lineked_;
		GLuint glId_;

		static GLuint compileShader(const char* code, GLenum type);
	};
};

