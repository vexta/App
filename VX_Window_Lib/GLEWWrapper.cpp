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

std::pair<GLuint, GLuint> vx_window_namespace_::GLEWWrapper::generateFramebufferObjectWithTexture(unsigned short width, unsigned short height) {
	GLuint fbo;
	GLuint texture;
	GLuint rbo;

	// generate famebuffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// generate texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	// generate renderbuffer
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach renderbuffer to framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// check framebuffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		throw (VX_Window_RunTimeError("Failed to create Framebuffer"));
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return std::make_pair(fbo, texture);
}

GLuint vx_window_namespace_::GLEWWrapper::generateFramebufferObject(unsigned short width, unsigned short height)
{
	GLuint fbo;
	GLuint rbo;

	// generate famebuffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// generate renderbuffer
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach renderbuffer to framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// check framebuffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		throw (VX_Window_RunTimeError("Failed to create Framebuffer"));
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}
