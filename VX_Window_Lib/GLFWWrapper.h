#pragma once

#include "VX_Window_RunTimeError.h"
#include <GLFW/glfw3.h>
#include <memory>

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
		bool voidBool;
	};

}
