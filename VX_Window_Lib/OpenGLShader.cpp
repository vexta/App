#include "OpenGLShader.h"

#include <Windows.h>
#include <iostream>

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
	WCHAR result[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, result);

	std::wcout << result << std::endl;

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

	if (result == GL_FALSE) {
		GLchar log[512];
		glGetProgramInfoLog(glId_, 512, NULL, log);
		throw OpenGLRuntimeError(log);
	}
	else {
		GLchar log[512];
		glGetProgramInfoLog(glId_, 512, NULL, log);
		std::cout << log << std::endl;
	}
	lineked_ = !(bool)result;
}

void vx_opengl_namespace_::OpenGLShader::deleteShader()
{
	if (lineked_) {
		glDeleteProgram(glId_);
	}
}

GLint vx_opengl_namespace_::OpenGLShader::getUniformLocation(const std::string & name)
{
	return glGetUniformLocation(glId_, name.c_str());
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLuint value)
{
	glUniform1ui(getUniformLocation(name), value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLuint valueA, const GLuint valueB)
{
	glUniform2ui(getUniformLocation(name), valueA, valueB);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLuint valueA, const GLuint valueB, const GLuint valueC)
{
	glUniform3ui(getUniformLocation(name), valueA, valueB, valueC);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLuint valueA, const GLuint valueB, const GLuint valueC, const GLuint valueD)
{
	glUniform4ui(getUniformLocation(name), valueA, valueB, valueC, valueD);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLint value)
{
	glUniform1i(getUniformLocation(name), value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLint valueA, const GLint valueB)
{
	glUniform2i(getUniformLocation(name), valueA, valueB);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLint valueA, const GLint valueB, const GLint valueC)
{
	glUniform3i(getUniformLocation(name), valueA, valueB, valueC);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLint valueA, const GLint valueB, const GLint valueC, const GLint valueD)
{
	glUniform4i(getUniformLocation(name), valueA, valueB, valueC, valueD);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLfloat value)
{
	glUniform1f(getUniformLocation(name), value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLfloat valueA, const GLfloat valueB)
{
	glUniform2f(getUniformLocation(name), valueA, valueB);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLfloat valueA, const GLfloat valueB, const GLfloat valueC)
{
	glUniform3f(getUniformLocation(name), valueA, valueB, valueC);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValue(const std::string & name, const GLfloat valueA, const GLfloat valueB, const GLfloat valueC, const GLfloat valueD)
{
	glUniform4f(getUniformLocation(name), valueA, valueB, valueC, valueD);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec1(const std::string & name, const GLsizei count, const GLuint * value)
{
	glUniform1uiv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec2(const std::string & name, const GLsizei count, const GLuint * value)
{
	glUniform2uiv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec3(const std::string & name, const GLsizei count, const GLuint * value)
{
	glUniform3uiv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec4(const std::string & name, const GLsizei count, const GLuint * value)
{
	glUniform4uiv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec1(const std::string & name, const GLsizei count, const GLint * value)
{
	glUniform1iv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec2(const std::string & name, const GLsizei count, const GLint * value)
{
	glUniform2iv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec3(const std::string & name, const GLsizei count, const GLint * value)
{
	glUniform3iv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec4(const std::string & name, const GLsizei count, const GLint * value)
{
	glUniform4iv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec1(const std::string & name, const GLsizei count, const GLfloat * value)
{
	glUniform1fv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec2(const std::string & name, const GLsizei count, const GLfloat * value)
{
	glUniform2fv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec3(const std::string & name, const GLsizei count, const GLfloat * value)
{
	glUniform3fv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueVec4(const std::string & name, const GLsizei count, const GLfloat * value)
{
	glUniform4fv(getUniformLocation(name), count, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat2(const std::string & name, const GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix2fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat3(const std::string & name, const GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix3fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUnifromValueMat4(const std::string & name, const GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat2x3(const std::string & name, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix2x3fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat3x2(const std::string & name, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix3x2fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat2x4(const std::string & name, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix2x4fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat4x2(const std::string & name, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix4x2fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat3x4(const std::string & name, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix3x4fv(getUniformLocation(name), count, transpose, value);
}

void vx_opengl_namespace_::OpenGLShader::setUniformValueMat4x3(const std::string & name, GLsizei count, GLboolean transpose, const GLfloat * value)
{
	glUniformMatrix4x3fv(getUniformLocation(name), count, transpose, value);
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

	if (result) { // TODO there is a problem somewhere - shader result is GL_FALSE, but shader works
		GLchar log[512];
		glGetShaderInfoLog(glid, 512, NULL, log);
		std::cout << log << std::endl;	
		std::cout.flush();
		throw OpenGLRuntimeError(log);
	}
	return glid;
}
