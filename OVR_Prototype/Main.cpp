#include <iostream>
#include <VX_Window_Lib.h>

int main() {
	vxWnd::OpenGLWindow wnd;

	auto keyCallback = [&wnd](int key, int action) {
		if (key == GLFW_KEY_ESCAPE)
			wnd.destroy();
	};

	auto mouseCallback = [&wnd](double xpos, double ypos) {
		std::cout << xpos << "\t" << ypos << std::endl;
	};

	wnd.setKeyCallback(keyCallback);
	wnd.setMousePosCallback(mouseCallback);
	wnd.create();

	while (!wnd.shouldClose()) {
		wnd.update();
	};

	std::cout << "hello oculus" << std::endl;
	return 0;
}