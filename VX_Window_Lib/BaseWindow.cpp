#include "BaseWindow.h"

vx_window_namespace_::BaseWindow::BaseWindow() :
	width_(800),
	height_(600),
	title_("VXWindow"),
	created_(new bool(false))
{
}

vx_window_namespace_::BaseWindow::BaseWindow(const unsigned short width, const unsigned short height, const std::string & title) :
	width_(width),
	height_(height),
	title_(title),
	created_(new bool(false))
{
}


vx_window_namespace_::BaseWindow::~BaseWindow()
{
}

void vx_window_namespace_::BaseWindow::setWidth(const unsigned short width)
{
	width_ = width;
}

unsigned short vx_window_namespace_::BaseWindow::getWidth() const
{
	return width_;
}

void vx_window_namespace_::BaseWindow::setHeight(const unsigned short height)
{
	height_ = height;
}

unsigned short vx_window_namespace_::BaseWindow::getHeight() const
{
	return height_;
}

void vx_window_namespace_::BaseWindow::setTitle(const std::string & title)
{
	title_ = title;
}

std::string vx_window_namespace_::BaseWindow::getTitle() const
{
	return title_;
}

bool vx_window_namespace_::BaseWindow::isCreated() const
{
	return created_.get();
}

void vx_window_namespace_::BaseWindow::setKeyCallback(std::function<void(int, int)> keyCallback)
{
	keyCallback_ = keyCallback;
}

void vx_window_namespace_::BaseWindow::setMousePosCallback(std::function<void(double, double)> mousePosCallback)
{
	mousePosCallback_ = mousePosCallback;
}

void vx_window_namespace_::BaseWindow::setMouseButtonCallback(std::function<void(int, int, int)> mouseButtonCallback)
{
	mouseButtonCallback_ = mouseButtonCallback;
}
