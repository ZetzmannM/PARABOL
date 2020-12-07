#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <string>

#include "ChannelPrintStream.h"
#include "Surface.h"
#include "Timer.h"

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
	//++++ TO TEST: - Timer.cpp, Surface.cpp ++++ 
	TimeHandler hnd = TimeHandler(true, 30, 20);

	Surface::Window w = Surface::Window({ 100,300,100,300 }, NULL, {}, "TestWindow");

	//++++ TEST OVER ++++ 
	while (!glfwWindowShouldClose(w.getGLFWHandle())) {
		hnd.start();
		// Render stuff in here theoretically
		PRINT_DEBUG("RENDER");
		hnd.stop();
		hnd.vsync();
		glfwSwapBuffers(w.getGLFWHandle());
		glfwPollEvents();
	}

	return 0;
}