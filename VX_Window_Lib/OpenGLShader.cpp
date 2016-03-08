#include "OpenGLShader.h"



vx_opengl_namespace_::OpenGLShader::OpenGLShader()
{
}


vx_opengl_namespace_::OpenGLShader::~OpenGLShader()
{
}

void vx_opengl_namespace_::OpenGLShader::create()
{
	glId_ = glCreateProgram();
}

void vx_opengl_namespace_::OpenGLShader::attachShader(const std::string & code, GLenum type)
{
	switch (type) {
	case GL_VERTEX_SHADER:
		vertexCode_ = code;
	case GL_FRAGMENT_SHADER:
		fragmentCode_ = code;
	}
}

void vx_opengl_namespace_::OpenGLShader::attachShaderFile(const std::string & path, GLenum type)
{
	std::ifstream file;
	file.exceptions(std::ifstream::badbit);
	try {
		file.open(path.c_str());
		std::stringstream fileStream;
		fileStream << file.rdbuf();
		file.close();
		attachShader(fileStream.str(), type);
	}
	catch (std::ifstream::failure e) {
		throw OpenGLRuntimeError(e.what());
	}
}

void vx_opengl_namespace_::OpenGLShader::compileAndLink()
{
	GLint result;
	GLuint vertexId = compileShader(vertexCode_.c_str(), GL_VERTEX_SHADER);
	GLuint fragmentId = compileShader(fragmentCode_.c_str(), GL_FRAGMENT_SHADER);

	glAttachShader(glId_, vertexId);
	glAttachShader(glId_, fragmentId);
	glLinkProgram(glId_);
	glGetProgramiv(glId_, GL_LINK_STATUS, &result);

	if (result) {
		GLchar log[512];
		glGetProgramInfoLog(glId_, 512, NULL, log);
		throw OpenGLRuntimeError(log);
	}
	lineked_ = !(bool)result;
}

void vx_opengl_namespace_::OpenGLShader::deleteShader()
{
	if (lineked_) {
		glDeleteProgram(glId_);
	}
}

vx_opengl_namespace_::OpenGLShader::operator GLuint()
{
	return glId_;
}

GLuint vx_opengl_namespace_::OpenGLShader::compileShader(const char * code, GLenum type)
{
	GLint result;
	GLuint glid = glCreateShader(type);
	glShaderSource(glid, 1, &code, NULL);
	glCompileShader(type);
	glGetShaderiv(glid, GL_COMPILE_STATUS, &result);

	if (result) {
		GLchar log[512];
		glGetShaderInfoLog(glid, 512, NULL, log);
		throw OpenGLRuntimeError(log);
	}
	return glid;
}
