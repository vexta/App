#pragma once

#include "VX_Window_RunTimeError.h"

#define GLEW_STATIC
#include <GL\glew.h>
#include "GLEWWrapper.h"

#include <memory>

namespace vx_window_namespace_ {
	class GLEWWrapper
	{
	public:
		~GLEWWrapper();
		void initialize();
		bool isInitialized() const;

		static std::shared_ptr<GLEWWrapper> getInstance();

		static std::pair<GLuint, GLuint> generateFramebufferObjectWithTexture(unsigned short width, unsigned short height);
	private:
		GLEWWrapper();
		
		bool initialized_;
	};
};

