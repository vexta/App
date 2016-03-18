#include <VX_OVR_Lib.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


std::shared_ptr<vxOvr::OVRHMDHandle> ovrHmdHandle;
GLuint cubeVAO, floorVAO;

glm::mat4 cube1Model, cube2Model, cube3Model, cube4Model, floorModel;
vxOpenGL::OpenGLShader shader;

// temp only
glm::mat4 view;
glm::mat4 projection;

void init() {
	auto ovr = vxOvr::OVRWrapper::getInstance();
	ovr->initialize();

	ovrHmdHandle = ovr->getOVRHMDDeviceHandle();
	ovrHmdHandle->setWindowParams(1920 / 2, 1080 / 2, "OVR Prototype");
	ovrHmdHandle->initialize();

	GLfloat cube_vertices[] = {

		// rear side
		-0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,

		// front side
		-0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,

		// left side
		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,

		// right side
		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,

		// bottom side
		-0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,

		// top side
		-0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,
	};

	GLfloat floor_vertices[] = {
		-1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,

		1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f
	};

	GLuint cubeVBO, floorVBO;

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);

	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);

	glBindVertexArray(floorVAO);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	cube1Model = glm::translate(glm::mat4(1), glm::vec3(-2.0f, 0.0f, -2.0f));
	cube2Model = glm::translate(glm::mat4(1), glm::vec3(-2.0f, 0.0f, 2.0f));
	cube3Model = glm::translate(glm::mat4(1), glm::vec3(2.0f, 0.0f, 2.0f));
	cube4Model = glm::translate(glm::mat4(1), glm::vec3(2.0f, 0.0f, -2.0f));
	floorModel = glm::translate(glm::scale(glm::mat4(1), glm::vec3(5.0f, 1.0f, 5.0f)), glm::vec3(0.0f, -0.5f, 0.0f));

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	projection = glm::perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 20.0f);

	try {
		shader.create();
		shader.attachShaderFile("..//Resources//OVRPrototype//window_quad_vertex", GL_VERTEX_SHADER);
		shader.attachShaderFile("..//Resources//OVRProtoype//window_quad_fragment", GL_FRAGMENT_SHADER);
		shader.compileAndLink();
	}
	catch (std::exception e) {
		std::cout << e.what << std::endl;
	}
}

void render(ovrEyeType eye) {
	auto fbo = ovrHmdHandle->prepareFramebuffer(eye);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	eye == ovrEye_Left ? glClearColor(0.0f, 0.0f, 0.3f, 1.0f) : glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	glUseProgram(shader);
	auto modelLoc = glGetUniformLocation(shader, "model");
	auto viewLoc = glGetUniformLocation(shader, "view");
	auto projectionLoc = glGetUniformLocation(shader, "projection");
	auto colorLoc = glGetUniformLocation(shader, "color");
	auto light1Loc = glGetUniformLocation(shader, "light1");
	auto light2Loc = glGetUniformLocation(shader, "light2");

	glUseProgram(0);
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