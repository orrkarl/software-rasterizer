#include "predef.h"

#include "converters.h"
#include "util.h"

#include "user_data.h"

void parseArguments(const std::vector<std::string>& args, bool& renderOnce) {
	renderOnce = false;
	for (const auto& arg : args) {
		if (arg == "-once") {
			renderOnce = true;
		}
	}
}

int main(int argc, const char** argv) {
	const uvec2 VIEWPORT{1280, 720};
	const float DEPTH_BUFFER_CLEAR = std::numeric_limits<float>::max();
	const Color COLOR_BUFFER_CLEAR = {0, 0, 0, 1};
	bool renderOnce;
	parseArguments(std::vector<std::string>(argv, argv + argc), renderOnce);
	std::vector<float> depthBuffer(VIEWPORT.x * VIEWPORT.y, DEPTH_BUFFER_CLEAR);
	std::vector<Color> colorBuffer(VIEWPORT.x * VIEWPORT.y, COLOR_BUFFER_CLEAR);
	std::chrono::milliseconds frameWait(30);

	auto window = setupGlfw(VIEWPORT.x, VIEWPORT.y);
	if (!window) {
		std::cerr << "could not init GLFW" << std::endl;
		return EXIT_FAILURE;
	}

	init(VIEWPORT);
	
	auto lastFrameLogTime = std::chrono::high_resolution_clock::now();
	auto rendered = false;
	while (!glfwWindowShouldClose(window)) {
		if (renderOnce && rendered) {
			glfwPollEvents();
			continue;
		}
		
		auto start = std::chrono::high_resolution_clock::now();
		std::fill(depthBuffer.begin(), depthBuffer.end(), DEPTH_BUFFER_CLEAR);
		std::fill(colorBuffer.begin(), colorBuffer.end(), COLOR_BUFFER_CLEAR);

		periodic(VIEWPORT, depthBuffer.data(), colorBuffer.data());
		glDrawPixels(VIEWPORT.x, VIEWPORT.y, GL_RGBA, GL_UNSIGNED_BYTE, colorBuffer.data());

		glfwSwapBuffers(window);
		glfwPollEvents();
		auto end = std::chrono::high_resolution_clock::now();
		if ((end - lastFrameLogTime) > std::chrono::milliseconds(500)) {
			std::cout << "Frame rendering took " << ((end - start).count()) / 1000 << "us" << std::endl;
			lastFrameLogTime = end;
		}
		rendered = true;
		std::this_thread::sleep_for(frameWait);		
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
