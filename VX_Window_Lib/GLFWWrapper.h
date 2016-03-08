#pragma once

#include <exception>
#include <memory>

#include <GLFW/glfw3.h>

#include "VX_Window_RunTimeError.h"

namespace vx_window_namespace_ {

	class GLFWWrapper
	{
	public:
		~GLFWWrapper();
		void initialize();
		bool isInitialized() const;

		static std::shared_ptr<GLFWWrapper> getInstance();
	private:
		GLFWWrapper();

		bool initialized_;
	};

}
