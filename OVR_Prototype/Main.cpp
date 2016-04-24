#include <VX_OVR_Lib.h>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	glBindVertexArray(vao);

	shader.setUniformValueMat4("model", 1, GL_FALSE, glm::value_ptr(model));
	shader.setUniformValueMat3("normalMatrix", 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(model)))));
	shader.setUniformValue("object.ambient", ambient.x, ambient.y, ambient.z);
	shader.setUniformValue("object.diffuse", diffuse.x, diffuse.y, diffuse.z);
	shader.setUniformValue("object.specular", specular.x, specular.y, specular.z);
	glDrawArrays(GL_TRIANGLES, 0, verticesCnt);

	glBindVertexArray(0);
}


std::shared_ptr<vxOvr::OVRHMDHandle> ovrHmdHandle;
GLuint cubeVAO, floorVAO, sphereVAO;
vxOpenGL::OpenGLShader shader;

bool pressedKeys[1024];
Viewer viewer;
sceneObject objects[5]; // spheres 0, 2; cubes 1, 3; floor 4;

std::vector<float> generateIcosphere(int iterations) {
	struct Triangle {
		Triangle(int v1, int v2, int v3) : v1_(v1), v2_(v2), v3_(v3) { }
		size_t v1_, v2_, v3_;
	};

	std::vector<glm::vec3> vertices;
	std::list<Triangle> triangles;

	// zero iteration -> vertices are common corners of orthogonal rectangles
	float t = (1.0 + glm::sqrt(5.0)) / 2.0;

	vertices.push_back(glm::normalize(glm::vec3(-1.0, t, 0.0)));
	vertices.push_back(glm::normalize(glm::vec3(1.0, t, 0.0)));
	vertices.push_back(glm::normalize(glm::vec3(-1.0, -t, 0.0)));
	vertices.push_back(glm::normalize(glm::vec3(1.0, -t, 0.0)));

	vertices.push_back(glm::normalize(glm::vec3(0.0, -1.0, t)));
	vertices.push_back(glm::normalize(glm::vec3(0.0, 1.0, t)));
	vertices.push_back(glm::normalize(glm::vec3(0.0, -1.0, -t)));
	vertices.push_back(glm::normalize(glm::vec3(0.0, 1.0, -t)));

	vertices.push_back(glm::normalize(glm::vec3(t, 0, -1.0)));
	vertices.push_back(glm::normalize(glm::vec3(t, 0, 1.0)));
	vertices.push_back(glm::normalize(glm::vec3(-t, 0, -1.0)));
	vertices.push_back(glm::normalize(glm::vec3(-t, 0, 1.0)));

	triangles.push_back(Triangle(0, 11, 5));
	triangles.push_back(Triangle(0, 5, 1));
	triangles.push_back(Triangle(0, 1, 7));
	triangles.push_back(Triangle(0, 7, 10));
	triangles.push_back(Triangle(0, 10, 11));

	triangles.push_back(Triangle(1, 5, 9));
	triangles.push_back(Triangle(5, 11, 4));
	triangles.push_back(Triangle(11, 10, 2));
	triangles.push_back(Triangle(10, 7, 6));
	triangles.push_back(Triangle(7, 1, 8));
	
	triangles.push_back(Triangle(3, 9, 4));
	triangles.push_back(Triangle(3, 4, 2));
	triangles.push_back(Triangle(3, 2, 6));
	triangles.push_back(Triangle(3, 6, 8));
	triangles.push_back(Triangle(3, 8, 9));
	
	triangles.push_back(Triangle(4, 9, 5));
	triangles.push_back(Triangle(2, 4, 11));
	triangles.push_back(Triangle(6, 2, 10));
	triangles.push_back(Triangle(8, 6, 7));
	triangles.push_back(Triangle(9, 8, 1));

	auto middlePoint = [&](int i1, int i2) {
		static std::map<long long, int> cache;
		long long smallerIndex = i1 < i2 ? i1 : i2;
		long long greaterIndex = i1 > i2 ? i1 : i2;
		long long key = (smallerIndex << 32) + greaterIndex;

		auto cached = cache.find(key);
		if (cached != cache.end()) {
			return cached->second;
		}
		else {
			vertices.push_back(glm::normalize((vertices[i1] + vertices[i2]) / 2.0f));
			int index = vertices.size() - 1;
			cache[key] = index;
			return index;
		}
	};

	int i;
	for (i = 0; i < iterations; i++) {
		std::list<Triangle> triangles2;

		std::for_each(triangles.begin(), triangles.end(), [&](Triangle &triangle) {
		
			int ia = middlePoint(triangle.v1_, triangle.v2_);
			int ib = middlePoint(triangle.v2_, triangle.v3_);
			int ic = middlePoint(triangle.v3_, triangle.v1_);

			triangles2.push_back(Triangle(triangle.v1_, ia, ic));
			triangles2.push_back(Triangle(triangle.v2_, ib, ia));
			triangles2.push_back(Triangle(triangle.v3_, ic, ib));
			triangles2.push_back(Triangle(ia, ib, ic));
		});
		triangles = triangles2;
	}

	std::vector<float> sphere(9 * triangles.size());
	i = 0;
	for (auto iter = triangles.begin(); iter != triangles.end(); iter++) {
		sphere[i++] = vertices[iter->v1_].x;
		sphere[i++] = vertices[iter->v1_].y;
		sphere[i++] = vertices[iter->v1_].z;

		sphere[i++] = vertices[iter->v2_].x;
		sphere[i++] = vertices[iter->v2_].y;
		sphere[i++] = vertices[iter->v2_].z;

		sphere[i++] = vertices[iter->v3_].x;
		sphere[i++] = vertices[iter->v3_].y;
		sphere[i++] = vertices[iter->v3_].z;
	}

	return sphere;
};

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
	ovrHmdHandle->setWindowParams(1920 / 2, 1080 / 2, "OVR Prototype");
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

	try {
		shader.create();
		shader.attachShaderFile("..//Resources//OVRPrototype//vertex_shader", GL_VERTEX_SHADER);
		shader.attachShaderFile("..//Resources//OVRPrototype//fragment_shader", GL_FRAGMENT_SHADER);
		shader.compileAndLink();
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
	}

	std::vector<GLfloat> cube_vertices{

		// rear side
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		// front side
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		// left side
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		// right side
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,

		// bottom side
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		// top side
		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	};
	std::vector<GLfloat> cube_normals{
		// rear side
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,

		// front side
		0.0f,  0.0f,  1.0f,
		0.0f,  0.0f,  1.0f,
		0.0f,  0.0f,  1.0f,
		0.0f,  0.0f,  1.0f,
		0.0f,  0.0f,  1.0f,
		0.0f,  0.0f,  1.0f,

		// left side
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,

		// right side
		1.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,

		// bottom side
		0.0f, -1.0f,  0.0f,
		0.0f, -1.0f,  0.0f,
		0.0f, -1.0f,  0.0f,
		0.0f, -1.0f,  0.0f,
		0.0f, -1.0f,  0.0f,
		0.0f, -1.0f,  0.0f,

		// top side
		0.0f,  1.0f,  0.0f,
		0.0f,  1.0f,  0.0f,
		0.0f,  1.0f,  0.0f,
		0.0f,  1.0f,  0.0f,
		0.0f,  1.0f,  0.0f,
		0.0f,  1.0f,  0.0f,
	};
	std::vector<GLfloat> floor_vertices{
		-1.0f,  0.0f, -1.0f,
		-1.0f,  0.0f,  1.0f,
		1.0f,  0.0f, -1.0f,

		1.0f,  0.0f,  1.0f,
		1.0f,  0.0f, -1.0f,
		-1.0f,  0.0f,  1.0f,
	};
	std::vector<GLfloat> floor_normals{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	GLuint cubeVBO, cubeNormalsVBO, floorVBO, floorNormalsVBO, sphereVBO;

	glUseProgram(shader);
	auto positionLoc = shader.getAttribLocation("position");
	auto normalLoc = shader.getAttribLocation("normal");

	// cube
	glGenVertexArrays(1, &cubeVAO);	
	glBindVertexArray(cubeVAO);

	// positions
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(GLfloat), cube_vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionLoc);

	// normals
	glGenBuffers(1, &cubeNormalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, cube_normals.size() * sizeof(GLfloat), cube_normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normalLoc);

	glBindVertexArray(0);

	// floor
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);
	
	// positions
	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, floor_vertices.size() * sizeof(GLfloat), floor_vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionLoc);

	// normals
	glGenBuffers(1, &floorNormalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, floor_normals.size() * sizeof(GLfloat), floor_normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normalLoc);

	glBindVertexArray(0);

	// sphere
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	auto sphereVertices = generateIcosphere(3);

	// positions
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionLoc);

	// normals - same as positions
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normalLoc);

	glBindVertexArray(0);

	objects[0].model = glm::translate(glm::vec3(-2.0f, 0.0f, -2.0f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	objects[1].model = glm::translate(glm::vec3(-2.0f, 0.0f, 2.0f));
	objects[2].model = glm::translate(glm::vec3(2.0f, 0.0f, 2.0f)) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	objects[3].model = glm::translate(glm::vec3(2.0f, 0.0f, -2.0f));
	objects[4].model = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f)) * glm::scale(glm::vec3(5.0f, 1.0f, 5.0f));
	
	objects[0].vao = sphereVAO;
	objects[1].vao = cubeVAO;
	objects[2].vao = sphereVAO;
	objects[3].vao = cubeVAO;
	objects[4].vao = floorVAO;

	objects[0].verticesCnt = sphereVertices.size() / 3;
	objects[1].verticesCnt = cube_vertices.size() / 3;
	objects[2].verticesCnt = sphereVertices.size() / 3;
	objects[3].verticesCnt = cube_vertices.size() / 3;
	objects[4].verticesCnt = floor_vertices.size() / 3;

	objects[0].ambient = glm::vec3(0.2f, 0.0f, 0.0f);
	objects[0].diffuse = glm::vec3(0.4f, 0.0f, 0.0f);
	objects[0].specular = glm::vec3(0.2f, 0.0f, 0.0f);

	objects[1].ambient = glm::vec3(0.0f, 0.2f, 0.0f);
	objects[1].diffuse = glm::vec3(0.0f, 0.4f, 0.0f);
	objects[1].specular = glm::vec3(0.0f, 0.2f, 0.0f);

	objects[2].ambient = glm::vec3(0.0f, 0.0f, 0.2f);
	objects[2].diffuse = glm::vec3(0.0f, 0.0f, 0.4f);
	objects[2].specular = glm::vec3(0.0f, 0.0f, 0.2f);

	objects[3].ambient = glm::vec3(0.2f, 0.2f, 0.0f);
	objects[3].diffuse = glm::vec3(0.4f, 0.4f, 0.0f);
	objects[3].specular = glm::vec3(0.2f, 0.2f, 0.0f);

	objects[4].ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	objects[4].diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	objects[4].specular = glm::vec3(0.2f, 0.2f, 0.2f);
}

void render(ovrEyeType eye) {
	auto fbo = ovrHmdHandle->prepareFramebuffer(eye);
	ovrHmdHandle->setViewport(eye);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	glUseProgram(shader);

	auto projection = ovrHmdHandle->getProjectionMatrix(eye);

	shader.setUniformValueMat4("projection", 1, GL_TRUE, (float*)&projection);

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

	shader.setUniformValue("viewPos", viewer.position.x, viewer.position.y + ovrHmdHandle->getUserHeight(), viewer.position.z);

	for (auto i = 0; i < 5; i++) {
		objects[i].render(shader);
	}
	glUseProgram(0);
}


int main() {
	init();

	double t, t0 = glfwGetTime();
	
	while (!ovrHmdHandle->shouldClose()) {
		t = glfwGetTime();

		processKeyInput(t - t0);
		ovrHmdHandle->getTrackingState();
		render(ovrEye_Left);
		render(ovrEye_Right);
		ovrHmdHandle->submitFrame();

		t0 = t;
	}

	return 0;
}