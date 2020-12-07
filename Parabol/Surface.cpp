#include "Surface.h"

#include "ChannelPrintStream.h"

using namespace Surface;

bool Window::init = false;

Window::Window(WindowBounds bounds, GLFWmonitor* hmon, const WindowProperties& properties, const std::string& title) {
	this->prop = properties;
	this->title = title;

	if(!init) {
		glfwInit();
		init = true;
	}

	glfwWindowHint(GLFW_RED_BITS, properties.colorBits / 3);
	glfwWindowHint(GLFW_GREEN_BITS, properties.colorBits / 3);
	glfwWindowHint(GLFW_BLUE_BITS, properties.colorBits / 3);
	glfwWindowHint(GLFW_ALPHA_BITS, 0); //Option?
	glfwWindowHint(GLFW_DEPTH_BITS, properties.depthBits);
	glfwWindowHint(GLFW_STENCIL_BITS, properties.stencilBits);
	glfwWindowHint(GLFW_SAMPLES, properties.msaaCount);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, properties.resizeable);
	glfwWindowHint(GLFW_DECORATED, properties.decorated);
	
	this->hWnd = glfwCreateWindow((bounds.right - bounds.left), (bounds.top - bounds.bottom), title.c_str(), NULL, NULL);

	if (!this->hWnd) {
		PRINT_ERR("Window Creation Failed! (OGL_FAIL|GLFW_FAIL)", CH_SEVERITY_HALT, CHANNEL_DEBUG);
	}

	glfwSetWindowPos(this->hWnd, bounds.left, bounds.bottom);
}
Window::~Window() {
	glfwDestroyWindow(this->hWnd);
}
void Window::show(){
	this->visible = true;
	glfwShowWindow(this->hWnd);
}
void Window::hide(){
	this->visible = false;
	glfwHideWindow(this->hWnd);
}
glm::i32vec2 Window::getWindowPosition() const {
	int x, y;
	glfwGetWindowPos(this->hWnd, &x, &y);
	return { x,y };
}
glm::i32vec2 Window::getWindowSize() const {
	int x, y;
	glfwGetWindowSize(this->hWnd, &x, &y);
	return { x,y };
}
glm::i32vec2 Window::toWindowRelativeCoordinates(const glm::i32vec2& ref) const {
	return ref - getWindowPosition();
}
glm::i32vec2 Window::toScreenRelativeCoordinates(const glm::i32vec2& ref) const {
	return ref + getWindowPosition();
}
GLFWwindow* Window::getGLFWHandle() {
	return this->hWnd;
}
WindowProperties Window::getProperties() const {
	return this->prop;
}
void Window::hideCursor(){
	glfwSetInputMode(this->hWnd, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
void Window::showCursor(){
	glfwSetInputMode(this->hWnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
