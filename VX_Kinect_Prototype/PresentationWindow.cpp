#include "stdafx.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../VX_Kinect_Lib/KinectParameters.h"
#include "../VX_Kinect_Lib/KinectTypes.h"
#include "../VX_Kinect_Lib/KinectData.h"
#include "../VX_Kinect_Lib/KinectFacade.h"
#include "../VX_Kinect_Lib/KinectHelper.h"
#include "DisplayTypes.h"
#include "PresentationWindow.h"

DisplayModes PresentationWindow::mode = DisplayModes::Color;
bool PresentationWindow::saveMesh = false;
KinectParameters PresentationWindow::myParameters;

PresentationWindow::PresentationWindow()
	: applicationTitle("Kinect Module Presentation")
{ }

int main()
{
	PresentationWindow application;
	application.DrawWindow();
	return EXIT_SUCCESS;
}

void PresentationWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		mode = DisplayModes::Color;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		mode = DisplayModes::Depth;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		mode = DisplayModes::Infrared;
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		mode = DisplayModes::Surface;
	//if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	//	mode = DisplayModes::Skeleton;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		saveMesh = true;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		myParameters.minimumDepth -= 0.5f;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		myParameters.minimumDepth += 0.5f;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		myParameters.maximumDepth -= 0.5f;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		myParameters.maximumDepth += 0.5f;
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		CycleValues(myParameters.reconstructionParameters.voxelCountX);
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		CycleValues(myParameters.reconstructionParameters.voxelCountY);
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		CycleValues(myParameters.reconstructionParameters.voxelCountZ);
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		CycleValues(myParameters.reconstructionParameters.voxelsPerMeter);
}

void PresentationWindow::DrawKinectData()
{
	KinectData myData;
	KinectTypes myTypes = KinectTypes::ColorData | KinectTypes::InfraredData | KinectTypes::DepthData | KinectTypes::BodyData | KinectTypes::SurfaceData | KinectTypes::MeshData | KinectTypes::ColorMeshData;

	kinectFacade->GetKinectData(myData, myTypes, myParameters);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);

	if (myData.colorData && mode == DisplayModes::Color) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, KinectFacade::colorWidth, KinectFacade::colorHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)myData.colorData->pFrameBuffer->pBits);
	if (myData.depthData && mode == DisplayModes::Depth) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, KinectFacade::width, KinectFacade::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)myData.depthData->pFrameBuffer->pBits);
	if (myData.infraredData && mode == DisplayModes::Infrared) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, KinectFacade::width, KinectFacade::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)myData.infraredData->pFrameBuffer->pBits);
	if (myData.infraredData && mode == DisplayModes::Surface) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, KinectFacade::width, KinectFacade::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)myData.surfaceData->pFrameBuffer->pBits);

	if (myData.meshData && saveMesh)
	{
		KinectHelper::WriteAsciiObjMeshFile(myData.meshData, "C:\\Work\\test.obj", false);
		saveMesh = false;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(KinectFacade::colorWidth, 0, 0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(KinectFacade::colorWidth, KinectFacade::colorHeight, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0, KinectFacade::colorHeight, 0.0f);
	glEnd();
}

void PresentationWindow::PrintHelp()
{
	std::cout << "Kinect Presentation Module" << std::endl;
	std::cout << "--------------------------" << std::endl;
	std::cout << " " << std::endl;
	std::cout << "Kinect presentation module and Kinect API module created by Mario Csaplar, xcsaplarm@is.stuba.sk" << std::endl;
	std::cout << "Team 16 (Vexta), summer semester 2015/2016 at FIIT STU" << std::endl;
	std::cout << " " << std::endl;
	std::cout << "Modes:" << std::endl;
	std::cout << "1 - color map (1920x1080)" << std::endl;
	std::cout << "2 - depth map (512x424)" << std::endl;
	std::cout << "3 - infrared map (512x424)" << std::endl;
	std::cout << "4 - surface map (512x424)" << std::endl;
	std::cout << "5 - skeleton (disabled)" << std::endl;
	std::cout << " " << std::endl;
	std::cout << "Properties:" << std::endl;
	std::cout << "m - cycle through voxels per meter (128/256/384/512/640)" << std::endl;
	std::cout << "s - save mesh to default location" << std::endl;
	std::cout << "x - cycle through X-axis voxel resolution (128/256/384/512/640)" << std::endl;
	std::cout << "y - cycle through Y-axis voxel resolution (128/256/384/512/640)" << std::endl;
	std::cout << "z - cycle through Z-axis voxel resolution (128/256/384/512/640)" << std::endl;
	std::cout << "left arrow  - decrease minimum depth" << std::endl;
	std::cout << "right arrow - increase minimum depth" << std::endl;
	std::cout << "up arrow    - increase maximum depth" << std::endl;
	std::cout << "down arrow  - decrease maximum depth" << std::endl;
}

void PresentationWindow::DrawWindow()
{
	// first initialize GLFW
	if (!glfwInit()) exit(EXIT_FAILURE);

	// then initialize the window
	GLFWwindow* window;
	window = glfwCreateWindow(KinectFacade::colorWidth, KinectFacade::colorHeight, applicationTitle.c_str(), NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, KeyCallback);
	glewInit();

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

	// Initialize textures
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// OpenGL setup
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	glEnable(GL_TEXTURE_2D);

	// Camera setup
	glViewport(0, 0, KinectFacade::colorWidth, KinectFacade::colorHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, KinectFacade::colorWidth, KinectFacade::colorHeight, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);

	// print help
	PrintHelp();

	while (!glfwWindowShouldClose(window))
	{
		DrawKinectData();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);

	glfwTerminate();
}

