#include <VX_OVR_Lib.h>
#include <iostream>

void init() {

}

void render() {

}


int main() {
	vxWnd::OpenGLStereoWindow wnd;
	wnd.create();
	
	std::pair<GLuint, GLuint> fbLeft = vxWnd::GLEWWrapper::generateFramebufferObjectWithTexture(800, 600);
	std::pair<GLuint, GLuint> fbRight = vxWnd::GLEWWrapper::generateFramebufferObjectWithTexture(800, 600);
	

	while (!wnd.shouldClose()) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbLeft.first);
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, fbRight.first);
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		wnd.update(fbLeft.second, fbRight.second);
	}
	return 0;
}