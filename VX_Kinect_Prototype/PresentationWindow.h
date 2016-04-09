#pragma once
class PresentationWindow
{
	KinectFacade *kinectFacade;
	const std::string applicationTitle;
	static DisplayModes mode;
	static bool saveMesh;
	static KinectParameters myParameters;

	GLuint textureId;
	void DrawKinectData();
	void PrintHelp();

	template<typename T>
	static void CycleValues(T &value)
	{
		if (value == 640)
		{
			value = 128;
		}
		else
		{
			value += 128;
		}
	}
public:
	PresentationWindow();
	static void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
	void DrawWindow();
};

