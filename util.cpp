#include "util.h"

#include <iostream>

void errorCallback(int error, const char* description) {
    std::cerr << "Error: " <<  description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

GLFWwindow* setupGlfw(unsigned int w, unsigned int h) {
	if (!glfwInit()) {
		std::cerr << "could not init GLFW" << std::endl;
		return nullptr;
	}
	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	auto window = glfwCreateWindow(w, h, "software rasterization", nullptr, nullptr);
	if (!window) {
		return window;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);

	return window;
}
