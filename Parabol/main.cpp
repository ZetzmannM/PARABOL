#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif

	glfwInit();

	GLFWwindow* hndl = glfwCreateWindow(200, 200, "Test", NULL, NULL);
	while (!glfwWindowShouldClose(hndl)) {

		glfwSwapBuffers(hndl);
		glfwPollEvents();
	}
	glfwDestroyWindow(hndl);
	return 0;
}