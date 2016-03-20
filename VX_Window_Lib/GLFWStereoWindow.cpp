#include "GLFWStereoWindow.h"

vx_window_namespace_::GLFWStereoWindow::GLFWStereoWindow()
{
}

vx_window_namespace_::GLFWStereoWindow::GLFWStereoWindow(const unsigned short width, const unsigned short height, const std::string title) :
	GLFWWindow(width, height, title)
{
}


vx_window_namespace_::GLFWStereoWindow::~GLFWStereoWindow()
{
}

void vx_window_namespace_::GLFWStereoWindow::create()
{
	GLFWWindow::create();
	quadShader_.create();
	quadShader_.attachShaderFile("..//Resources//GLFWStereoWindow//window_quad_vertex", GL_VERTEX_SHADER);
	quadShader_.attachShaderFile("..//Resources//GLFWStereoWindow//window_quad_fragment", GL_FRAGMENT_SHADER);
	quadShader_.compileAndLink();

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	GLuint quadVBO;
	glGenVertexArrays(1, &quadVAO_);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO_);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);
	
}

void vx_window_namespace_::GLFWStereoWindow::update(GLuint leftTexture, GLuint rightTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width_, height_);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);


	glUseProgram(quadShader_);
	glBindVertexArray(quadVAO_);

	GLint mode_uniform = glGetUniformLocation(quadShader_, "mode");

	glUniform1i(mode_uniform, 1);
	glBindTexture(GL_TEXTURE_2D, leftTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform1i(mode_uniform, 2);
	glBindTexture(GL_TEXTURE_2D, rightTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);

	/*glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);*/

	GLFWWindow::update();
}

void vx_window_namespace_::GLFWStereoWindow::destroy()
{
	quadShader_.deleteShader();
	glDeleteVertexArrays(1, &quadVAO_);
	GLFWWindow::destroy();
}
