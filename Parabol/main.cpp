#include <GLFW/glfw3.h>

#include "ChannelPrintStream.h"

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif

	glfwInit();
	GLFWwindow* hndl = glfwCreateWindow(200, 200, "Test", NULL, NULL);

	//++++ TEST CHANNELPRINTSTREAM ++++ 
	PRINT("Test String", CHANNEL_DEBUG);
	PRINT("Test2", CHANNEL_VULKAN);
	PRINT("Test3", CHANNEL_VULKAN_DEBUG);
	PRINT("Test4", CHANNEL_GLFW);

	PRINT_ERR("Error1", CH_SEVERITY_HINT, CHANNEL_DEBUG);
	PRINT_ERR("Error2", CH_SEVERITY_WARNING, CHANNEL_DEBUG);
	try{
		PRINT_ERR("Error3", CH_SEVERITY_HALT, CHANNEL_DEBUG);
	}
	catch (std::exception& std) {	
		PRINT_DEBUG("CAUGHT1");
	}

	try {
		ASSERT(false, "Success", CHANNEL_DEBUG);
	}
	catch (std::exception& std) { 
		PRINT_DEBUG("CAUGHT2");
	}

	COND_INFO(false, "This worked!", CHANNEL_VULKAN);

	int a = 5;
	int* aptr = &a;

	PRINT_DEBUG(PTRSTR(aptr));
	PRINT_DEBUG(DEVPTRSTR(aptr));

	PRINT_DEBUG("End Test");
	//++++ TEST OVER ++++ 


	while (!glfwWindowShouldClose(hndl)) {
		glfwSwapBuffers(hndl);
		glfwPollEvents();
	}
	
	glfwDestroyWindow(hndl);
	

	return 0;
}