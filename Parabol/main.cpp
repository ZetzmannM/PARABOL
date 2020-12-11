#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <string>

#include "ChannelPrintStream.h"
#include "Surface.h"
#include "Timer.h"
#include "Env.h"
#include "inc_setting.h"

#ifdef ENV_WINDOWS
#include <Windows.h>
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include "VkUtils.h"

#ifdef ENV_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		TimeHandler handl = TimeHandler(true, 60, 20);
		
		Surface::Window w = Surface::Window({ 100,300,100,300 }, NULL, {}, "TestWindow");
		VK::VulkanInstanceInfo inf;
		inf.window = w.getGLFWHandle();
		inf.appInfo.name = "Test App";

		VK::VulkanInterface::vkInit(inf);

		//++++ TEST OVER ++++ 
		while (!glfwWindowShouldClose(w.getGLFWHandle())) {
			handl.start();
			handl.stop();
			handl.vsync();
			glfwSwapBuffers(w.getGLFWHandle());
			glfwPollEvents();
		}
	}
	glfwTerminate();
	VK::VulkanInterface::destroyInstance();
	
	return 0;
}