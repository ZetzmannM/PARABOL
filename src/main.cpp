#include <iostream>
#include <glm/matrix.hpp>
#include <GLFW/glfw3.h>

/// @brief Test Struct for Doxygen tests
struct Test {
	/// @brief help
	int a=0;

	/// @brief constructor
	Test() {

	}
	
	/// @brief Leaks memory because, ehh, I mean why not
	void leakMemory() {
		new int[200];
	}
};

/// @brief Simple main Function
/// @return 0, hopefully
int main() {
	glm::mat2 p;

	glfwInit();

	GLFWwindow* hndl = glfwCreateWindow(200, 200, "Test", NULL, NULL);
	while (!glfwWindowShouldClose(hndl)) {
		glfwPollEvents();
	}
	glfwDestroyWindow(hndl);
	
	std::cout << "Hello" << std::endl;
	system("pause");
	return 0;
}