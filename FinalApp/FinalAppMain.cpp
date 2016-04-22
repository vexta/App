#include <VX_OVR_Lib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
GLuint floorVAO, cubeVAO, cubeVBO, kinectMeshVAO, kinectMeshVBO, kinectSingleHandMeshVAO, kinectSingleHandMeshVBO, sphereVAO, sphereVBO;

glm::mat4 floorModel;
vxOpenGL::OpenGLShader shader, kinectShader;

bool pressedKeys[1024];
KinectFacade *kinectFacade;
//VX_Network_Lib::KniznicaDLL komunikacia;
KinectParameters parameters;

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

	shader.setUniformValueMat4("model", 1, GL_FALSE, glm::value_ptr(model));
	shader.setUniformValue("object.ambient", ambient.x, ambient.y, ambient.z);
	shader.setUniformValue("object.diffuse", diffuse.x, diffuse.y, diffuse.z);
	shader.setUniformValue("object.specular", specular.x, specular.y, specular.z);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, verticesCnt);
	glBindVertexArray(0);
}

Viewer viewer;
sceneObject object, kinectMesh, kinectSingleHandMesh, headPos, leftHandPos, rightHandPos, cube1, cube2, cube3, sphere;


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

	//cubes
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

	//kinect body parts
	glGenVertexArrays(1, &kinectMeshVAO);
	glGenBuffers(1, &kinectMeshVBO);

	glGenVertexArrays(1, &kinectSingleHandMeshVAO);
	glGenBuffers(1, &kinectSingleHandMeshVBO);

	//sphere
	//std::vector<glm::vec4> sphere_vertices;
	//std::vector<glm::vec3> sphere_normals;
	//std::vector<GLushort> sphere_elements;

	//load_obj("sphere/sphere.obj", sphere_vertices, sphere_normals, sphere_elements);
	//
	//float *sphereVertices = new float[3*sizeof(sphere_vertices)];
	//int i = 0;

	//for (std::vector<glm::vec4>::iterator it = sphere_vertices.begin(); it != sphere_vertices.end(); ++it){
	//	sphereVertices[i] = (*it).x;
	//	sphereVertices[i+1] = (*it).y;
	//	sphereVertices[i+2] = (*it).z;
	//	i += 3;
	//}

	//printf("sphere vertex 1: %f", sphereVertices[0]);
	//printf("sphere vertex n: %f", sphereVertices[3 * sizeof(sphere_vertices)-1]);

	//glBindVertexArray(sphereVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices) * 3 * sizeof(float), sphereVertices, GL_STREAM_DRAW);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	//glEnableVertexAttribArray(0);
	//glBindVertexArray(0);


	object.model = glm::translate(glm::scale(glm::mat4(1), glm::vec3(5.0f, 1.0f, 5.0f)), glm::vec3(0.0f, -0.5f, 0.0f));

	object.vao = floorVAO;
	object.verticesCnt = 6;

	object.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	object.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
	object.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	kinectMesh.model = glm::mat4(1);
	kinectMesh.vao = kinectMeshVAO;

	kinectMesh.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	kinectMesh.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	kinectMesh.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	kinectSingleHandMesh.model = glm::mat4(1);
	kinectSingleHandMesh.vao = kinectSingleHandMeshVAO;

	kinectSingleHandMesh.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	kinectSingleHandMesh.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	kinectSingleHandMesh.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	headPos.model = glm::mat4(1);
	headPos.vao = cubeVAO;
	headPos.verticesCnt = 36;

	headPos.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	headPos.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	headPos.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	rightHandPos.model = glm::mat4(1);
	rightHandPos.vao = cubeVAO;
	rightHandPos.verticesCnt = 36;

	rightHandPos.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	rightHandPos.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	rightHandPos.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	leftHandPos.model = glm::mat4(1);
	leftHandPos.vao = cubeVAO;
	leftHandPos.verticesCnt = 36;

	leftHandPos.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	leftHandPos.diffuse = glm::vec3(0.45f, 0.96f, 0.078f);
	leftHandPos.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	//cube1
	cube1.model = glm::mat4(1);
	cube1.vao = cubeVAO;
	cube1.verticesCnt = 36;

	cube1.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube1.diffuse = glm::vec3(0.0f, 0.5f, 0.5f);
	cube1.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	//cube2
	cube2.model = glm::mat4(1);
	cube2.vao = cubeVAO;
	cube2.verticesCnt = 36;

	cube2.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube2.diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
	cube2.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	//cube3
	cube3.model = glm::mat4(1);
	cube3.vao = cubeVAO;
	cube3.verticesCnt = 36;

	cube3.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube3.diffuse = glm::vec3(0.5f, 0.5f, 0.0f);
	cube3.specular = glm::vec3(0.2f, 0.2f, 0.2f);

	//sphere
	sphere.position = glm::vec3(0.22f, 1.0f, -0.7f);

	sphere.model = glm::mat4(1);
	sphere.vao = sphereVAO;
	sphere.verticesCnt = 36;

	sphere.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	sphere.diffuse = glm::vec3(0.5f, 0.5f, 0.0f);
	sphere.specular = glm::vec3(0.2f, 0.2f, 0.2f);

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

	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);
	//eye == ovrEye_Left ? glClearColor(0.0f, 0.0f, 0.3f, 1.0f) : glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
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
	headPos.render(shader);
	leftHandPos.render(shader);
	rightHandPos.render(shader);
	cube1.render(shader);
	cube2.render(shader);
	cube3.render(shader);
	kinectMesh.render(kinectShader);
	kinectSingleHandMesh.render(kinectShader);
	//sphere.render(shader);
	
	glUseProgram(0);
}

int main() {
	init();
	
	double t, t0 = glfwGetTime();
	int grippedObjectIdRight = 0, grippedObjectIdLeft = 0;	//cislo objektu, ktory je prave uchopeny

	cube1.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-0.15f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //fialova
	cube1.position = glm::vec3(-0.15f, 1.4f, -0.5f);
	cube2.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //modra
	cube2.position = glm::vec3(0.0f, 1.4f, -0.5f);
	cube3.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.15f, 1.4f, -0.5f)), glm::vec3(0.07, 0.07, 0.07)); //zlta
	cube3.position = glm::vec3(0.15f, 1.4f, -0.5f);

	boolean leftHandGripped = false;
	float lastHandPosition = 0.0f;

	while (!ovrHmdHandle->shouldClose()) {
		//if (komunikacia.newDataAvailable())		//zisti ci mas nove aktualne data
		//	printf("%d \n",komunikacia.Get());						//ak mas nove data tak ichy ziskaj
		
		t = glfwGetTime();

		processKeyInput(t - t0);
		//view = glm::lookAt(viewer.position, viewer.position + viewer.front, viewer.worldUp);

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

		if (data.bodies)
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
					printf("ruka zavreta\n");
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
				
				printf("%f %f %f\n", handLeftPosition.X, handLeftPosition.Y, handLeftPosition.Z);
				
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

		if (data.meshData) {	// if mesh data successfully retrieved
			const Vector3 *vertices = nullptr, *normals = nullptr;
			data.meshData->GetVertices(&vertices);
			data.meshData->GetNormals(&normals);
			int vertexCount = data.meshData->VertexCount();

			kinectMesh.verticesCnt = vertexCount;
			kinectMesh.model = glm::translate(glm::rotate(glm::mat4(1), 180.0f, glm::vec3(1, 0, 0)), glm::vec3(0.0, -2.0, 0.0));

			//kinectMesh.model = glm::rotate(glm::mat4(1), 180.0f, glm::vec3(0, 0, 1));


			glBindVertexArray(kinectMeshVAO);
			glBindBuffer(GL_ARRAY_BUFFER, kinectMeshVBO);
			glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STREAM_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);

			//komunikacia.Send(data.meshData->VertexCount, data.meshData->GetVertices);		//posli nove ziskane data   //pole vektorov a ich dlzku
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