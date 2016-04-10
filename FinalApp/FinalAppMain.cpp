#include <VX_OVR_Lib.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <NuiKinectFusionApi.h>
#include <KinectParameters.h>
#include <KinectTypes.h>
#include <KinectData.h>
#include <KinectFacade.h>
#include <KinectHelper.h>

std::shared_ptr<vxOvr::OVRHMDHandle> ovrHmdHandle;
GLuint floorVAO, kinectVAO, kinectVBO;

glm::mat4 floorModel;
vxOpenGL::OpenGLShader shader, kinectShader;

bool pressedKeys[1024];
KinectFacade *kinectFacade;

struct sceneObject {
	glm::mat4 model;
	glm::vec3 ambient, diffuse, specular;
	GLuint vao;
	unsigned int verticesCnt;

	void render(vxOpenGL::OpenGLShader &shader);
};

struct Viewer {
	enum class Direction {
		Forward,
		Backward,
		Left,
		Right
	};

	enum class Rotation {
		Positive,
		Negative
	};

	void move(Direction dir, float deltaTime = 1.0f) {
		float v = velocity * deltaTime;
		switch (dir)
		{
		case Viewer::Direction::Forward:
			position += front * v;
			break;
		case Viewer::Direction::Backward:
			position -= front * v;
			break;
		case Viewer::Direction::Left:
			position -= right * v;
			break;
		case Viewer::Direction::Right:
			position += right * v;
			break;
		}
	}

	void rotate(Rotation rotation, float deltaTime = 1.0f) {

		switch (rotation) {
		case Rotation::Positive:
			yaw += deltaTime * rotationSpeed;
			break;
		case Rotation::Negative:
			yaw -= deltaTime * rotationSpeed;
			break;
		}
		glm::vec3 newFront(-sin(glm::radians(yaw)), 0.0f, -cos(glm::radians(yaw)));
		front = glm::normalize(newFront);
		right = glm::normalize(glm::cross(newFront, worldUp));
	}

	glm::vec3 position;

	glm::vec3 worldUp;
	glm::vec3 front;
	glm::vec3 right;

	float yaw;
	float velocity;
	float rotationSpeed;
};

void sceneObject::render(vxOpenGL::OpenGLShader &shader) {
	glUseProgram(shader);

	shader.setUniformValueMat4("model", 1, GL_FALSE, glm::value_ptr(model));
	shader.setUniformValue("object.ambient", ambient.x, ambient.y, ambient.z);
	shader.setUniformValue("object.diffuse", diffuse.x, diffuse.y, diffuse.z);
	shader.setUniformValue("object.specular", specular.x, specular.y, specular.z);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, verticesCnt);
	glBindVertexArray(0);
}

Viewer viewer;
sceneObject object, kinectObject;

void processKeyInput(float deltaTime) {
	if (pressedKeys[GLFW_KEY_W]) {
		viewer.move(Viewer::Direction::Forward, deltaTime);
	}
	if (pressedKeys[GLFW_KEY_S]) {
		viewer.move(Viewer::Direction::Backward, deltaTime);
	}
	if (pressedKeys[GLFW_KEY_A]) {
		viewer.move(Viewer::Direction::Left, deltaTime);
	}
	if (pressedKeys[GLFW_KEY_D]) {
		viewer.move(Viewer::Direction::Right, deltaTime);
	}
	if (pressedKeys[GLFW_KEY_Q]) {
		viewer.rotate(Viewer::Rotation::Positive, deltaTime);
	}
	if (pressedKeys[GLFW_KEY_E]) {
		viewer.rotate(Viewer::Rotation::Negative, deltaTime);
	}
}

void init() {
	auto ovr = vxOvr::OVRWrapper::getInstance();
	ovr->initialize();

	viewer.position = glm::vec3(0.0f, 0.0f, 0.0f);
	viewer.front = glm::vec3(0.0f, 0.0f, -1.0f);
	viewer.right = glm::vec3(1.0f, 0.0f, 0.0f);
	viewer.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	viewer.yaw = 0.0f;
	viewer.velocity = 3.0f;
	viewer.rotationSpeed = 60.0f;

	ovrHmdHandle = ovr->getOVRHMDDeviceHandle();
	ovrHmdHandle->setWindowParams(1600, 900, "OVR Prototype");
	ovrHmdHandle->initialize();

	auto keyCallback = [](int key, int action) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			ovrHmdHandle->setShouldClose(true);
			break;
		}

		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	};

	ovrHmdHandle->setKeyCallback(keyCallback);

	GLfloat floor_vertices[] = {
		-1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,

		1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f
	};

	GLuint floorVBO;

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

	GLfloat MeshData[] = {
		0.1f, 0.2f, 0.3f, 0.4f
	};

	//kinect
	glGenVertexArrays(1, &kinectVAO);
	glGenBuffers(1, &kinectVBO);

	glBindBuffer(GL_ARRAY_BUFFER, kinectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshData), MeshData, GL_STREAM_DRAW);

	glBindVertexArray(kinectVAO);

	glBindVertexArray(0);

	object.model = glm::translate(glm::scale(glm::mat4(1), glm::vec3(5.0f, 1.0f, 5.0f)), glm::vec3(0.0f, -0.5f, 0.0f));

	object.vao = floorVAO;
	object.verticesCnt = 6;

	object.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	object.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	object.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	kinectObject.model = glm::mat4();
	kinectObject.vao = kinectVAO;
	kinectObject.verticesCnt = 1;

	kinectObject.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	kinectObject.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	kinectObject.specular = glm::vec3(0.2f, 0.2f, 0.2f);


	try {
		shader.create();
		shader.attachShaderFile("..//Resources//FinalApp//vertex_shader", GL_VERTEX_SHADER);
		shader.attachShaderFile("..//Resources//FinalApp//fragment_shader", GL_FRAGMENT_SHADER);
		shader.compileAndLink();

		kinectShader.create();
		kinectShader.attachShaderFile("..//Resources//FinalApp//KinectMeshVertex.txt", GL_VERTEX_SHADER);
		kinectShader.attachShaderFile("..//Resources//FinalApp//KinectMeshFragment.txt", GL_FRAGMENT_SHADER);
		kinectShader.compileAndLink();
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
	}

	// then attempt to construct kinect processor
	try
	{
		kinectFacade = new KinectFacade();
	}
	// device failure
	catch (std::domain_error exception)
	{
		std::cout << exception.what() << std::endl;
	}
}

void render(ovrEyeType eye) {
	auto fbo = ovrHmdHandle->prepareFramebuffer(eye);
	ovrHmdHandle->setViewport(eye);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	eye == ovrEye_Left ? glClearColor(0.0f, 0.0f, 0.3f, 1.0f) : glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(shader);


	auto projection = ovrHmdHandle->getProjectionMatrix(eye);

	shader.setUniformValueMat4("projection", 1, GL_TRUE, (float*)&projection);
	//shader.setUniformValueMat4("view", 1, GL_FALSE, glm::value_ptr(view));

	auto view = ovrHmdHandle->getViewMatrix(eye,
		OVR::Vector3f(viewer.position.x, viewer.position.y, viewer.position.z),
		OVR::Vector3f(viewer.front.x, viewer.front.y, viewer.front.z),
		OVR::Vector3f(viewer.right.x, viewer.right.y, viewer.right.z),
		glm::radians(viewer.yaw));
	shader.setUniformValueMat4("view", 1, GL_TRUE, (float*)&view);

	shader.setUniformValue("light1.position", 5.0f, 3.0f, 5.0f);
	shader.setUniformValue("light1.ambient", 0.2f, 0.2f, 0.2f);
	shader.setUniformValue("light1.diffuse", 0.5f, 0.5f, 0.5f);
	shader.setUniformValue("light1.specular", 1.0f, 1.0f, 1.0f);

	shader.setUniformValue("light2.position", -5.0f, 3.0f, -5.0f);
	shader.setUniformValue("light2.ambient", 0.2f, 0.2f, 0.2f);
	shader.setUniformValue("light2.diffuse", 0.5f, 0.5f, 0.5f);
	shader.setUniformValue("light2.specular", 1.0f, 1.0f, 1.0f);

	glUseProgram(kinectShader);
	kinectShader.setUniformValueMat4("projection", 1, GL_TRUE, (float*)&projection);

	kinectShader.setUniformValueMat4("view", 1, GL_TRUE, (float*)&view);

	kinectShader.setUniformValue("light1.position", 5.0f, 3.0f, 5.0f);
	kinectShader.setUniformValue("light1.ambient", 0.2f, 0.2f, 0.2f);
	kinectShader.setUniformValue("light1.diffuse", 0.5f, 0.5f, 0.5f);
	kinectShader.setUniformValue("light1.specular", 1.0f, 1.0f, 1.0f);

	kinectShader.setUniformValue("light2.position", -5.0f, 3.0f, -5.0f);
	kinectShader.setUniformValue("light2.ambient", 0.2f, 0.2f, 0.2f);
	kinectShader.setUniformValue("light2.diffuse", 0.5f, 0.5f, 0.5f);
	kinectShader.setUniformValue("light2.specular", 1.0f, 1.0f, 1.0f);

	object.render(shader);
	//kinectObject.render(kinectShader);


	glUseProgram(0);
}

int main() {
	init();

	double t, t0 = glfwGetTime();
	while (!ovrHmdHandle->shouldClose()) {
		t = glfwGetTime();

		processKeyInput(t - t0);
		//view = glm::lookAt(viewer.position, viewer.position + viewer.front, viewer.worldUp);


		KinectData data;
		KinectParameters p;
		kinectFacade->GetKinectData(data, KinectTypes::BodyData, p);
		if (data.bodies && data.bodies[0])
		{
			auto abc = data.ExtractJointsForPerson(0);
			if(abc) std::cout << "x: " << abc->Position.X << " y: " << abc->Position.Y << " z: " << abc->Position.Z << std::endl;
		}

		if (data.meshData) {
			const Vector3 *vertices = nullptr;
			data.meshData->GetVertices(&vertices);
			int vertexCount = data.meshData->VertexCount();
			glBindVertexArray(kinectVAO);
			glBindBuffer(GL_ARRAY_BUFFER, kinectVBO);
			glBufferData(GL_ARRAY_BUFFER, vertexCount, vertices, GL_STREAM_DRAW);
			glBindVertexArray(0);
		}


		render(ovrEye_Left);
		render(ovrEye_Right);
		ovrHmdHandle->submitFrame();

		t0 = t;
	}
	return 0;
}