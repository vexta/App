#pragma once

#include <memory>

#define GLEW_STATIC
#include <GL\glew.h>

#include "VX_Window_RunTimeError.h"

namespace vx_window_namespace_ {
	class GLEWWrapper
	{
	public:
		~GLEWWrapper();
		void initialize();
		bool isInitialized() const;

		static std::shared_ptr<GLEWWrapper> getInstance();
	private:
		GLEWWrapper();
		
		bool initialized_;
	};
};

