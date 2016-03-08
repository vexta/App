#pragma once

#include <string>
#include <memory>
#include <functional>

namespace vx_window_namespace_ {

	class BaseWindow
	{
	public:
		BaseWindow();
		BaseWindow(const unsigned short width, const unsigned short height, const std::string &title);
		virtual ~BaseWindow();

		void setWidth(const unsigned short width);
		unsigned short getWidth() const;

		void setHeight(const unsigned short height);
		unsigned short getHeight() const;

		void setTitle(const std::string &title);
		std::string getTitle() const;

		virtual void create() = 0;
		virtual void update() = 0;
		virtual void destroy() = 0;
		virtual bool isCreated() const;
		virtual bool shouldClose() const = 0;

		virtual void setKeyCallback(std::function<void(int, int)> keyCallback);
		virtual void setMousePosCallback(std::function<void(double, double)> mousePosCallback);
		
	protected:
		unsigned short width_, height_;
		std::string title_;
		std::shared_ptr<bool> created_;  // shared between all copies of window

		std::function<void(int, int)> keyCallback_;
		std::function<void(double, double)> mousePosCallback_;

	};
};

