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

		GLint getUniformLocation(const std::string &name);

		void setUniformValue(const std::string &name, const GLuint value);
		void setUniformValue(const std::string &name, const GLuint valueA, const GLuint valueB);
		void setUniformValue(const std::string &name, const GLuint valueA, const GLuint valueB, const GLuint valueC);
		void setUniformValue(const std::string &name, const GLuint valueA, const GLuint valueB, const GLuint valueC, const GLuint valueD);

		void setUniformValue(const std::string &name, const GLint value);
		void setUniformValue(const std::string &name, const GLint valueA, const GLint valueB);
		void setUniformValue(const std::string &name, const GLint valueA, const GLint valueB, const GLint valueC);
		void setUniformValue(const std::string &name, const GLint valueA, const GLint valueB, const GLint valueC, const GLint valueD);

		void setUniformValue(const std::string &name, const GLfloat value);
		void setUniformValue(const std::string &name, const GLfloat valueA, const GLfloat valueB);
		void setUniformValue(const std::string &name, const GLfloat valueA, const GLfloat valueB, const GLfloat valueC);
		void setUniformValue(const std::string &name, const GLfloat valueA, const GLfloat valueB, const GLfloat valueC, const GLfloat valueD);

		void setUniformValueVec1(const std::string &name, const GLsizei count, const GLuint *value);
		void setUniformValueVec2(const std::string &name, const GLsizei count, const GLuint *value);
		void setUniformValueVec3(const std::string &name, const GLsizei count, const GLuint *value);
		void setUniformValueVec4(const std::string &name, const GLsizei count, const GLuint *value);

		void setUniformValueVec1(const std::string &name, const GLsizei count, const GLint *value);
		void setUniformValueVec2(const std::string &name, const GLsizei count, const GLint *value);
		void setUniformValueVec3(const std::string &name, const GLsizei count, const GLint *value);
		void setUniformValueVec4(const std::string &name, const GLsizei count, const GLint *value);

		void setUniformValueVec1(const std::string &name, const GLsizei count, const GLfloat *value);
		void setUniformValueVec2(const std::string &name, const GLsizei count, const GLfloat *value);
		void setUniformValueVec3(const std::string &name, const GLsizei count, const GLfloat *value);
		void setUniformValueVec4(const std::string &name, const GLsizei count, const GLfloat *value);

		
		void setUniformValueMat2(const std::string &name, const GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat3(const std::string &name, const GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat4(const std::string &name, const GLsizei count, GLboolean transpose, const GLfloat *value);

		void setUniformValueMat2x3(const std::string &name, GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat3x2(const std::string &name, GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat2x4(const std::string &name, GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat4x2(const std::string &name, GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat3x4(const std::string &name, GLsizei count, GLboolean transpose, const GLfloat *value);
		void setUniformValueMat4x3(const std::string &name, GLsizei count, GLboolean transpose, const GLfloat *value);

		/* FUTURE PLANS
		void collectAllUniforms();
		void bind();
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

