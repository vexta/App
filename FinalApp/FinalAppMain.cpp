#include <VX_OVR_Lib.h>
#include <iostream>
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
GLuint floorVAO, cubeVAO, cubeVBO, kinectMeshVAO, kinectMeshVBO;

glm::mat4 floorModel;
vxOpenGL::OpenGLShader shader, kinectShader;

bool pressedKeys[1024];
KinectFacade *kinectFacade;
//VX_Network_Lib::KniznicaDLL komunikacia;


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
sceneObject object, kinectMesh, headPos, leftHandPos, rightHandPos, cube;
std::vector<sceneObject> cubeArray;

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

	cube.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.22f, 1.4f, -0.7f)), glm::vec3(0.05, 0.05, 0.05));
	cube.position = glm::vec3(0.22f, 1.4f, -0.7f);

	cube.model = glm::mat4(1);
	cube.vao = cubeVAO;
	cube.verticesCnt = 36;

	cube.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	cube.diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
	cube.specular = glm::vec3(0.2f, 0.2f, 0.2f);


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
	cube.render(shader);
	kinectMesh.render(kinectShader);

	for (std::vector<sceneObject>::iterator it = cubeArray.begin(); it != cubeArray.end(); it++) {
		it->render(shader);
	}

	glUseProgram(0);
}

int main() {
	init();

	double t, t0 = glfwGetTime();

	cube.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.22f, 1.4f, -0.7f)), glm::vec3(0.05, 0.05, 0.05));
	boolean leftHandGripped = false;

	while (!ovrHmdHandle->shouldClose()) {
		//if (komunikacia.newDataAvailable())		//zisti ci mas nove aktualne data
		//	printf("%d \n",komunikacia.Get());						//ak mas nove data tak ichy ziskaj
		
		t = glfwGetTime();

		processKeyInput(t - t0);
		//view = glm::lookAt(viewer.position, viewer.position + viewer.front, viewer.worldUp);

		KinectData data;
		KinectParameters parameters;
		//parameters.reconstructionParameters.voxelsPerMeter = 256;
		//parameters.reconstructionParameters.voxelCountX = 512;
		//parameters.reconstructionParameters.voxelCountY = 512;
		//parameters.reconstructionParameters.voxelCountZ = 512;
		//
		//parameters.reconstructionParameters.voxelsPerMeter = 128;
		//parameters.reconstructionParameters.voxelCountX = 256;
		//parameters.reconstructionParameters.voxelCountY = 256;
		parameters.reconstructionParameters.voxelCountZ = 512;
		parameters.maximumDepth = 4.0;
		parameters.minimumDepth = 0.5;
		parameters.voxelStep = 2; // remove half of voxels
		//parameters.reconstructionParameters.voxelsPerMeter = 128;
		kinectFacade->GetKinectData(data, KinectTypes::MeshData | KinectTypes::BodyData, parameters);

		if (data.bodies)
		{
			Joint *jointsForFirstPerson = data.ExtractJointsForFirstPerson();
			if (jointsForFirstPerson)
			{
				const CameraSpacePoint headPosition = jointsForFirstPerson[JointType_Head].Position;
				const CameraSpacePoint handLeftPosition = jointsForFirstPerson[JointType_HandLeft].Position;
				const CameraSpacePoint handRightPosition = jointsForFirstPerson[JointType_HandRight].Position;

				glm::vec3 leftHandRelativeToHead = glm::vec3(headPosition.X, headPosition.Y, headPosition.Z) - glm::vec3(handLeftPosition.X, handLeftPosition.Y, handLeftPosition.Z);
				glm::vec3 rightHandRelativeToHead = glm::vec3(headPosition.X, headPosition.Y, headPosition.Z) - glm::vec3(handRightPosition.X, handRightPosition.Y, handRightPosition.Z);

				HandState handRightState, handLeftState;

				data.bodies[0]->get_HandRightState(&handRightState);
				data.bodies[0]->get_HandLeftState(&handLeftState);

				auto userHeight = ovrHmdHandle->getUserHeight();
				leftHandRelativeToHead.y -= userHeight;
				rightHandRelativeToHead.y -= userHeight;

				if (handRightState == HandState_Closed) {
					printf("ruka zavreta\n");
					rightHandPos.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);

					if (cube.position.x + 0.05 > -rightHandRelativeToHead.x && cube.position.x - 0.05 < -rightHandRelativeToHead.x &&
						cube.position.y + 0.05 > -rightHandRelativeToHead.y && cube.position.y - 0.05 < -rightHandRelativeToHead.y &&
						cube.position.z + 0.05 > -rightHandRelativeToHead.z && cube.position.z - 0.05 < -rightHandRelativeToHead.z) {

						cube.model = glm::scale(glm::translate(glm::mat4(1), -rightHandRelativeToHead), glm::vec3(0.05, 0.05, 0.05));
						cube.position = -rightHandRelativeToHead;
					}
				}

				if (handRightState == HandState_Open) {
					rightHandPos.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
				}

				if (handLeftState == HandState_Closed) {
					leftHandPos.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
					if (!leftHandGripped) {
						cubeArray.push_back(rightHandPos);
						leftHandGripped = true;
					}
				}

				if (handLeftState == HandState_Open) {
					leftHandPos.diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
					leftHandGripped = false;
				}
				
				headPos.model = glm::scale(glm::translate(glm::mat4(1), glm::vec3(headPosition.X, headPosition.Y + kinect_position_height, headPosition.Z)), glm::vec3(0.1, 0.1, 0.1));
				leftHandPos.model = glm::scale(glm::translate(glm::mat4(1), -leftHandRelativeToHead), glm::vec3(0.05, 0.05, 0.05));
				rightHandPos.model = glm::scale(glm::translate(glm::mat4(1), -rightHandRelativeToHead), glm::vec3(0.05, 0.05, 0.05));
				
				printf("%f %f %f\n", handLeftPosition.X, handLeftPosition.Y, handLeftPosition.Z);
				
			}
		}

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

	if (kinectFacade) free(kinectFacade);

	return 0;
}