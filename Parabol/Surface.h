#pragma once

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <functional>

#include "inttypes.h"

namespace Surface {

	struct WindowBounds {
		long left, right, bottom, top;
	};

	struct WindowProperties {
		uint8 depthBits = 24, stencilBits = 8, colorBits = 32;
		uint8 msaaCount = 1;
		bool decorated = true;
		bool resizeable = true;
	};

	/// @brief Basic Interface for a window, Only Basic things are presented here, since GLFW does a REALLY good job at the rest. 
	struct Window {
	private:
		WindowProperties prop;

		GLFWwindow* hWnd = nullptr;
		GLFWcursor* blankCursor = nullptr;

		std::string title;

		bool visible = true;
		
		static bool init;
		
	public:

		Window(WindowBounds pos, GLFWmonitor* hmon, const WindowProperties& properties, const std::string& title);
		~Window();

		void show();
		void hide();
			
		void hideCursor();
		void showCursor();

		WindowProperties getProperties() const;

		glm::i32vec2 getWindowPosition() const;
		glm::i32vec2 getWindowSize() const;

		/// @brief Conversion of Coordinates
		/// @return The new Coordinates relative to the Window
		glm::i32vec2 toWindowRelativeCoordinates(const glm::i32vec2& ref) const;

		/// @brief Conversion of Coordinates
		/// @return The new Coordinates relative to the Screen
		glm::i32vec2 toScreenRelativeCoordinates(const glm::i32vec2& ref) const;

		GLFWwindow* getGLFWHandle();
	};

}