#include <VX_OVR_Lib.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <NuiKinectFusionApi.h>
#include <KinectParameters.h>
#include <KinectTypes.h>
#include <KinectData.h>
#include <KinectFacade.h>
#include <KinectHelper.h>
#include <VX_Network_Lib.h>

const float kinect_position_height(0.78f);

std::shared_ptr<vxOvr::OVRHMDHandle> ovrHmdHandle;
GLuint	floorVAO, 
		cubeVAO, 
		sphereVAO, 
		kinectMeshVAO, 
		kinectMeshVBO, 
		kinectNormalVBO,
		kinectSingleHandMeshVAO, 
		kinectSingleHandMeshVBO;

vxOpenGL::OpenGLShader shader;

bool pressedKeys[1024];

KinectFacade *kinectFacade;
KinectParameters parameters;

VX_Network_Lib::KniznicaDLL komunikacia;		//pridane

int recievedVertexCount = 0;
int recievedNormalCount = 0;

struct sceneObject {
	glm::mat4 model;
	glm::vec3 position;
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

Viewer viewer;
sceneObject floorMesh, kinectMesh, 
			kinectSingleHandMesh, 
			headPos, leftHandPos, rightHandPos, 
			cube1, cube2, cube3, sphere;


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

void load_obj(const char* filename, std::vector<glm::vec4> &vertices, std::vector<glm::vec3> &normals, std::vector<GLushort> &elements)
{
	std::ifstream in(filename, std::ifstream::in);
	if (!in)
	{
		std::cerr << "Cannot open " << filename << std::endl; 
		exit(1);
	}

	std::string line;
	while (getline(in, line))
	{
		if (line.substr(0, 2) == "v ")
		{
			std::istringstream s(line.substr(2));
			glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0f;
			vertices.push_back(v);
		}
		else if (line.substr(0, 2) == "f ")
		{
			std::istringstream s(line.substr(2));
			GLushort a, b, c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			elements.push_back(a); elements.push_back(b); elements.push_back(c);
		}
		else if (line[0] == '#')
		{
			// ignoring this line 
		}
		else
		{
			// ignoring this line 
		}
	}

	/*normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
	for (int i = 0; i < elements.size(); i += 3)
	{
		GLushort ia = elements[i];
		GLushort ib = elements[i + 1];
		GLushort ic = elements[i + 2];
		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
			glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
		normals[ia] = normals[ib] = normals[ic] = normal;
	}*/
}

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


	// kinect buffers
	glGenVertexArrays(1, &kinectMeshVAO);
	glBindVertexArray(kinectMeshVAO);
	
	glGenBuffers(1, &kinectMeshVBO);
	glBindBuffer(GL_ARRAY_BUFFER, kinectMeshVBO);
	glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(float), cube_vertices.data(), GL_STREAM_DRAW);

	glGenBuffers(1, &kinectNormalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, kinectNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, cube_normals.size() * sizeof(float), cube_normals.data(), GL_STREAM_DRAW);

	glBindVertexArray(0);

	floorMesh.model = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f)) * glm::scale(glm::vec3(5.0f, 1.0f, 5.0f));
	kinectMesh.model = glm::mat4(1);
	kinectSingleHandMesh.model = glm::mat4(1);
	headPos.model = glm::mat4(1);
	rightHandPos.model = glm::mat4(1);
	leftHandPos.model = glm::mat4(1);
	cube1.model = glm::translate(glm::vec3(-0.15f, 1.4f, -0.5f)) * glm::scale(glm::vec3(0.07, 0.07, 0.07));
	cube2.model = glm::translate(glm::vec3(0.0f, 1.4f, -0.5f)) * glm::scale(glm::vec3(0.07, 0.07, 0.07));
	cube3.model = glm::translate(glm::vec3(0.15f, 1.4f, -0.5f)) * glm::scale(glm::vec3(0.07, 0.07, 0.07));

	floorMesh.vao = floorVAO;
	kinectMesh.vao = kinectMeshVAO;
	kinectSingleHandMesh.vao = kinectSingleHandMeshVAO;
	headPos.vao = sphereVAO;
	rightHandPos.vao = sphereVAO;
	leftHandPos.vao = sphereVAO;
	cube1.vao = cubeVAO;
	cube2.vao = cubeVAO;
	cube3.vao = cubeVAO;

	floorMesh.verticesCnt = floor_vertices.size() / 3;
	headPos.verticesCnt = sphereVertices.size() / 3;
	rightHandPos.verticesCnt = sphereVertices.size() / 3;
	leftHandPos.verticesCnt = sphereVertices.size() / 3;
	cube1.verticesCnt = cube_vertices.size() / 3;
	cube2.verticesCnt = cube_vertices.size() / 3;
	cube3.verticesCnt = cube_vertices.size() / 3;

	floorMesh.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	floorMesh.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	floorMesh.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	kinectMesh.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	kinectMesh.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	kinectMesh.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	kinectSingleHandMesh.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	kinectSingleHandMesh.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	kinectSingleHandMesh.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	headPos.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	headPos.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	headPos.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	rightHandPos.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	rightHandPos.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	rightHandPos.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	leftHandPos.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	leftHandPos.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	leftHandPos.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	cube1.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube1.diffuse = glm::vec3(0.0f, 0.5f, 0.5f);
	cube1.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	cube2.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube2.diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
	cube2.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	cube3.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube3.diffuse = glm::vec3(0.5f, 0.5f, 0.0f);
	cube3.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	// attempt to construct kinect processor
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

	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);
	//eye == ovrEye_Left ? glClearColor(0.0f, 0.0f, 0.3f, 1.0f) : glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
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

	floorMesh.render(shader);
	headPos.render(shader);
	leftHandPos.render(shader);
	rightHandPos.render(shader);
	cube1.render(shader);
	cube2.render(shader);
	cube3.render(shader);
	kinectMesh.render(shader);
	kinectSingleHandMesh.render(shader);
	
	glUseProgram(0);
}

int random(int min, int max) {
	return min + rand() % (max - min);
}

int main() {
	init();

	glUseProgram(shader);
	auto positionLoc = shader.getAttribLocation("position");
	auto normalLoc = shader.getAttribLocation("normal");
		
	double t, t0 = glfwGetTime();
	int grippedObjectIdRight = 0, grippedObjectIdLeft = 0;	//cislo objektu, ktory je prave uchopeny

	
	//inicialzacia kociek - treba zistit co tym chcel basnik povedat
	cube1.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-0.15f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //fialova
	cube1.position = glm::vec3(-0.15f, 1.4f, -0.5f);
	cube2.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //modra
	cube2.position = glm::vec3(0.0f, 1.4f, -0.5f);
	cube3.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.15f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //zlta
	cube3.position = glm::vec3(0.15f, 1.4f, -0.5f);
	

	boolean leftHandGripped = false;
	float lastHandPosition = 0.0f;

	static int vyvolena_kocka = random(0, 3);

	while (!ovrHmdHandle->shouldClose()) {		
		t = glfwGetTime();
		processKeyInput(t - t0);

		// vyfarbenie kociek 
		if (!komunikacia.isOculus()) {
			float lx, ly, lz,
				rx, ry, rz,
				k1x, k1y, k1z, k2x, k2y, k2z, k3x, k3y, k3z;

			komunikacia.GetKocky(&vyvolena_kocka, &lx, &ly, &lz, &rx, &ry, &rz, &k1x, &k1y, &k1z, &k2x, &k2y, &k2z, &k3x, &k3y, &k3z);
			cube1.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
			cube2.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
			cube3.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
			switch (vyvolena_kocka) {
			case 0: cube1.diffuse = glm::vec3(0.0f, 1.0f, 0.0f); break;
			case 1: cube2.diffuse = glm::vec3(0.0f, 1.0f, 0.0f); break;
			case 2: cube3.diffuse = glm::vec3(0.0f, 1.0f, 0.0f); break;
			default:
				break;
			}

			leftHandPos.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(lx, ly, -lz)), glm::vec3(0.07, 0.07, 0.07));
			rightHandPos.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(rx, ry, -rz)), glm::vec3(0.07, 0.07, 0.07));
			cube1.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(k1x, k1y, -k1z)), glm::vec3(0.07, 0.07, 0.07)); //zlta
			cube2.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(k2x, k2y, -k2z)), glm::vec3(0.07, 0.07, 0.07)); //zlta
			cube3.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(k3x, k3y, -k3z)), glm::vec3(0.07, 0.07, 0.07)); //zlta
		}


		KinectData data;
		//parameters.reconstructionParameters.voxelsPerMeter = 256;
		//parameters.reconstructionParameters.voxelCountX = 512;
		//parameters.reconstructionParameters.voxelCountY = 512;
		//parameters.reconstructionParameters.voxelCountZ = 512;
		//
		//parameters.reconstructionParameters.voxelsPerMeter = 128;
		//parameters.reconstructionParameters.voxelCountX = 256;
		//parameters.reconstructionParameters.voxelCountY = 256;
		parameters.reconstructionParameters.voxelCountZ = 512;
		
		if (!lastHandPosition)
		{
			parameters.maximumDepth = 4.0;
			parameters.minimumDepth = 0.5;
		}
		else
		{
			parameters.maximumDepth = lastHandPosition + 0.1;
			parameters.minimumDepth = lastHandPosition - 0.1;
		}

		parameters.voxelStep = 2; // remove half of voxels
		//parameters.reconstructionParameters.voxelsPerMeter = 128;
		kinectFacade->GetKinectData(data, KinectTypes::MeshData | KinectTypes::BodyData, parameters);

		if (data.bodies && komunikacia.isOculus())
		{
			int index = 0;
			Joint *jointsForFirstPerson = data.ExtractJointsForFirstPerson(index);
			if (jointsForFirstPerson)
			{
				const CameraSpacePoint headPosition = jointsForFirstPerson[JointType_Head].Position;
				const CameraSpacePoint handLeftPosition = jointsForFirstPerson[JointType_HandLeft].Position;
				const CameraSpacePoint handRightPosition = jointsForFirstPerson[JointType_HandRight].Position;

				glm::vec3 leftHandRelativeToHead = glm::vec3(headPosition.X, headPosition.Y, headPosition.Z) - glm::vec3(handLeftPosition.X, handLeftPosition.Y, handLeftPosition.Z);
				glm::vec3 rightHandRelativeToHead = glm::vec3(headPosition.X, headPosition.Y, headPosition.Z) - glm::vec3(handRightPosition.X, handRightPosition.Y, handRightPosition.Z);

				HandState handRightState, handLeftState;

				data.bodies[index]->get_HandRightState(&handRightState);
				data.bodies[index]->get_HandLeftState(&handLeftState);

				auto userHeight = ovrHmdHandle->getUserHeight();
				leftHandRelativeToHead.y -= userHeight;
				rightHandRelativeToHead.y -= userHeight;

				if (handRightState == HandState_Closed) {
					//printf("ruka zavreta\n");
					rightHandPos.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);

					if (cube1.position.x + 0.07 > -rightHandRelativeToHead.x && cube1.position.x - 0.07 < -rightHandRelativeToHead.x &&
						cube1.position.y + 0.07 > -rightHandRelativeToHead.y && cube1.position.y - 0.07 < -rightHandRelativeToHead.y &&
						cube1.position.z + 0.07 > -rightHandRelativeToHead.z && cube1.position.z - 0.07 < -rightHandRelativeToHead.z &&
						(grippedObjectIdRight == 0 || grippedObjectIdRight == 1)) {

						cube1.model = glm::scale(glm::translate(glm::mat4(1), -rightHandRelativeToHead), glm::vec3(0.07, 0.07, 0.07));
						cube1.position = -rightHandRelativeToHead;
						grippedObjectIdRight = 1;
					}
					if (cube2.position.x + 0.07 > -rightHandRelativeToHead.x && cube2.position.x - 0.07 < -rightHandRelativeToHead.x &&
						cube2.position.y + 0.07 > -rightHandRelativeToHead.y && cube2.position.y - 0.07 < -rightHandRelativeToHead.y &&
						cube2.position.z + 0.07 > -rightHandRelativeToHead.z && cube2.position.z - 0.07 < -rightHandRelativeToHead.z &&
						(grippedObjectIdRight == 0 || grippedObjectIdRight == 2)) {

						cube2.model = glm::scale(glm::translate(glm::mat4(1), -rightHandRelativeToHead), glm::vec3(0.07, 0.07, 0.07));
						cube2.position = -rightHandRelativeToHead;
						grippedObjectIdRight = 2;
					}
					if (cube3.position.x + 0.07 > -rightHandRelativeToHead.x && cube3.position.x - 0.07 < -rightHandRelativeToHead.x &&
						cube3.position.y + 0.07 > -rightHandRelativeToHead.y && cube3.position.y - 0.07 < -rightHandRelativeToHead.y &&
						cube3.position.z + 0.07 > -rightHandRelativeToHead.z && cube3.position.z - 0.07 < -rightHandRelativeToHead.z &&
						(grippedObjectIdRight == 0 || grippedObjectIdRight == 3)) {

						cube3.model = glm::scale(glm::translate(glm::mat4(1), -rightHandRelativeToHead), glm::vec3(0.07, 0.07, 0.07));
						cube3.position = -rightHandRelativeToHead;
						grippedObjectIdRight = 3;
					}
				}

				if (handRightState == HandState_Open) {
					rightHandPos.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
					grippedObjectIdRight = 0;
				}

				if (handLeftState == HandState_Closed) {
					leftHandPos.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);

					if (cube1.position.x + 0.07 > -leftHandRelativeToHead.x && cube1.position.x - 0.07 < -leftHandRelativeToHead.x &&
						cube1.position.y + 0.07 > -leftHandRelativeToHead.y && cube1.position.y - 0.07 < -leftHandRelativeToHead.y &&
						cube1.position.z + 0.07 > -leftHandRelativeToHead.z && cube1.position.z - 0.07 < -leftHandRelativeToHead.z &&
						(grippedObjectIdLeft == 0 || grippedObjectIdLeft == 1)) {

						cube1.model = glm::scale(glm::translate(glm::mat4(1), -leftHandRelativeToHead), glm::vec3(0.07, 0.07, 0.07));
						cube1.position = -leftHandRelativeToHead;
						grippedObjectIdLeft = 1;
					}
					if (cube2.position.x + 0.07 > -leftHandRelativeToHead.x && cube2.position.x - 0.07 < -leftHandRelativeToHead.x &&
						cube2.position.y + 0.07 > -leftHandRelativeToHead.y && cube2.position.y - 0.07 < -leftHandRelativeToHead.y &&
						cube2.position.z + 0.07 > -leftHandRelativeToHead.z && cube2.position.z - 0.07 < -leftHandRelativeToHead.z &&
						(grippedObjectIdLeft == 0 || grippedObjectIdLeft == 2)) {

						cube2.model = glm::scale(glm::translate(glm::mat4(1), -leftHandRelativeToHead), glm::vec3(0.07, 0.07, 0.07));
						cube2.position = -leftHandRelativeToHead;
						grippedObjectIdLeft = 2;
					}
					if (cube3.position.x + 0.07 > -leftHandRelativeToHead.x && cube3.position.x - 0.07 < -leftHandRelativeToHead.x &&
						cube3.position.y + 0.07 > -leftHandRelativeToHead.y && cube3.position.y - 0.07 < -leftHandRelativeToHead.y &&
						cube3.position.z + 0.07 > -leftHandRelativeToHead.z && cube3.position.z - 0.07 < -leftHandRelativeToHead.z &&
						(grippedObjectIdLeft == 0 || grippedObjectIdLeft == 3)) {

						cube3.model = glm::scale(glm::translate(glm::mat4(1), -leftHandRelativeToHead), glm::vec3(0.07, 0.07, 0.07));
						cube3.position = -leftHandRelativeToHead;
						grippedObjectIdLeft = 3;
					}
				}

				if (handLeftState == HandState_Open) {
					leftHandPos.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
					leftHandGripped = false;
					grippedObjectIdLeft = 0;
				}
				
				headPos.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(headPosition.X, headPosition.Y + kinect_position_height, headPosition.Z)), glm::vec3(0.1, 0.1, 0.1));
				leftHandPos.model = glm::scale(glm::translate(glm::mat4(1), -leftHandRelativeToHead), glm::vec3(0.05, 0.05, 0.05));
				rightHandPos.model = glm::scale(glm::translate(glm::mat4(1), -rightHandRelativeToHead), glm::vec3(0.05, 0.05, 0.05));
				
				//printf("%f %f %f\n", handLeftPosition.X, handLeftPosition.Y, handLeftPosition.Z);
				
				if (komunikacia.isOculus()) {
					komunikacia.Send(
						vyvolena_kocka,
						-leftHandRelativeToHead.x, -leftHandRelativeToHead.y, -leftHandRelativeToHead.z,
						-rightHandRelativeToHead.x, -rightHandRelativeToHead.y, -rightHandRelativeToHead.z,
						cube1.position.x, cube1.position.y, cube1.position.z,
						cube2.position.x, cube2.position.y, cube2.position.z,
						cube3.position.x, cube3.position.y, cube3.position.z
						);
					float treshhold_color = 1.5;
					float treshhold_reset = 1.7;

					if (cube1.position.y > treshhold_color)
						if (vyvolena_kocka == 0)
							cube1.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
						else
							cube1.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
					if (cube2.position.y > treshhold_color)
						if (vyvolena_kocka == 1)
							cube2.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
						else
							cube2.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
					if (cube3.position.y > treshhold_color)
						if (vyvolena_kocka == 2)
							cube3.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
						else
							cube3.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
					if (cube1.position.y > treshhold_reset || cube2.position.y > treshhold_reset || cube3.position.y > treshhold_reset) {
						vyvolena_kocka = random(0, 3);
						cube1.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-0.15f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //fialova
						cube1.position = glm::vec3(-0.15f, 1.4f, -0.5f);
						cube2.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //modra
						cube2.position = glm::vec3(0.0f, 1.4f, -0.5f);
						cube3.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.15f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //zlta
						cube3.position = glm::vec3(0.15f, 1.4f, -0.5f);

						cube1.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
						cube2.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
						cube3.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);

						grippedObjectIdLeft = 0;
						grippedObjectIdRight = 0;
					}
				}
			}
		}

		//if (data.bodies && data.meshData) {
		//	int index = 0;
		//	Joint *jointsForFirstPerson = data.ExtractJointsForFirstPerson(index);
		//	if (jointsForFirstPerson)
		//	{
		//		const CameraSpacePoint headPosition = jointsForFirstPerson[JointType_Head].Position;
		//		const CameraSpacePoint handLeftPosition = jointsForFirstPerson[JointType_HandLeft].Position;
		//		const CameraSpacePoint handRightPosition = jointsForFirstPerson[JointType_HandRight].Position;

		//		glm::vec3 leftHandRelativeToHead = glm::vec3(headPosition.X, headPosition.Y, headPosition.Z) - glm::vec3(handLeftPosition.X, handLeftPosition.Y, handLeftPosition.Z);
		//		glm::vec3 rightHandRelativeToHead = glm::vec3(headPosition.X, headPosition.Y, headPosition.Z) - glm::vec3(handRightPosition.X, handRightPosition.Y, handRightPosition.Z);

		//		HandState handRightState, handLeftState;

		//		data.bodies[index]->get_HandRightState(&handRightState);
		//		data.bodies[index]->get_HandLeftState(&handLeftState);

		//		auto userHeight = ovrHmdHandle->getUserHeight();
		//		rightHandRelativeToHead.y -= userHeight;

		//		// if mesh data successfully retrieved
		//		const Vector3 *vertices = nullptr, *normals = nullptr;
		//		data.meshData->GetVertices(&vertices);
		//		data.meshData->GetNormals(&normals);
		//		int vertexCount = data.meshData->VertexCount();

		//		float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX, maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN, avgX, avgY, avgZ;
		//		for (int i = 0; i < vertexCount; i++)
		//		{
		//			if (vertices[i].x < minX) minX = vertices[i].x;
		//			if (vertices[i].y < minY) minY = vertices[i].y;
		//			if (vertices[i].z < minZ) minZ = vertices[i].z;

		//			if (vertices[i].x > maxX) maxX = vertices[i].x;
		//			if (vertices[i].y > maxY) maxY = vertices[i].y;
		//			if (vertices[i].z > maxZ) maxZ = vertices[i].z;
		//		}

		//		avgX = (minX + maxX) / 2;
		//		avgY = (minY + maxY) / 2;
		//		avgZ = (minZ + maxZ) / 2;

		//		kinectSingleHandMesh.verticesCnt = vertexCount;
		//		kinectSingleHandMesh.model = glm::translate(glm::rotate(glm::translate(glm::mat4(1), glm::vec3(-avgX, -avgY, -avgZ)), 180.0f, glm::vec3(1, 0, 0)), -leftHandRelativeToHead);

		//		glBindVertexArray(kinectSingleHandMeshVAO);
		//		glBindBuffer(GL_ARRAY_BUFFER, kinectSingleHandMeshVBO);
		//		glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STREAM_DRAW);
		//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		//		glEnableVertexAttribArray(0);
		//		glBindVertexArray(0);

		//		lastHandPosition = handLeftPosition.Z;
		//	}
		//}

		if (komunikacia.isOculus()) {
			//if (data.meshData) {	// if mesh data successfully retrieved
				const Vector3 *vertices = nullptr, *normals = nullptr;
				//data.meshData->GetVertices(&vertices);
				//data.meshData->GetNormals(&normals);
				//int vertexCount = data.meshData->VertexCount();
				if (komunikacia.newDataAvailable()) {
					vertices = komunikacia.GetVrcholy(&recievedVertexCount, &recievedNormalCount, &normals);
					//printf("%d \n", recievedVertexCount);
					//data.meshData->GetVertices(&vertices);
					//recievedVertexCount = data.meshData->VertexCount();
				}

				kinectMesh.verticesCnt = recievedVertexCount;
				kinectMesh.model = glm::translate(glm::rotate(glm::mat4(1), 180.0f, glm::vec3(1, 0, 0)), glm::vec3(0, 1.0, 2.5));

				//kinectMesh.model = glm::rotate(glm::mat4(1), 180.0f, glm::vec3(0, 0, 1));

				glUseProgram(shader);
				glBindVertexArray(kinectMeshVAO);

				glBindBuffer(GL_ARRAY_BUFFER, kinectMeshVBO);
				glBufferData(GL_ARRAY_BUFFER, recievedVertexCount * 3 * sizeof(float), vertices, GL_STREAM_DRAW);
				glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(positionLoc);

				glBindBuffer(GL_ARRAY_BUFFER, kinectNormalVBO);
				glBufferData(GL_ARRAY_BUFFER, recievedNormalCount * 3 * sizeof(float), normals, GL_STREAM_DRAW);
				glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(normalLoc);

				glBindVertexArray(0);
			//}
		}
		else if (data.meshData) {
			const Vector3 *vertices = nullptr, *normals = nullptr;
			data.meshData->GetVertices(&vertices);
			data.meshData->GetNormals(&normals);
			int vertexCount = data.meshData->VertexCount();

			kinectMesh.verticesCnt = vertexCount;
			kinectMesh.model = glm::translate(glm::rotate(glm::mat4(1), 180.0f, glm::vec3(1, 0, 0)), glm::vec3(0.0, -2.0, 0.0));

			static int count = 0;
			//if (count++ > 2) {
			//	count = 0;
			//	printf("     %d \n", kinectMesh.verticesCnt);
			komunikacia.Send(data.meshData);		//posli nove ziskane data   //pole vektorov a ich dlzku
													//}
		}



		ovrHmdHandle->getTrackingState();

		render(ovrEye_Left);
		render(ovrEye_Right);
		ovrHmdHandle->submitFrame();

		t0 = t;
	}

	if (kinectFacade) delete kinectFacade;

	return 0;
}