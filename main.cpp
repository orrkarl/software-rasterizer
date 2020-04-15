#include "predef.h"

#include "converters.h"
#include "util.h"

#include "user_data.h"
#include "rasterizer.h"

std::vector<mat4> generateRandomTransforms() {
	std::vector<mat4> objects;

	const mat4 identity(1.f);
 
    auto M0 = translate(identity, glm::vec3(0, 0, 2.f));
    M0 = rotate(M0, glm::radians(45.f), glm::vec3(0, 1, 0));
 
    auto M1 = translate(identity, glm::vec3(-3.75f, 0, 0));
    M1 = rotate(M1, glm::radians(30.f), glm::vec3(1, 0, 0));
 
    auto M2 = translate(identity, glm::vec3(3.75f, 0, 0));
    M2 = rotate(M2, glm::radians(60.f), glm::vec3(0, 1, 0));
 
    auto M3 = glm::translate(identity, glm::vec3(0, 0, -2.f));
    M3 = rotate(M3, glm::radians(90.f), glm::vec3(0, 0, 1));

    objects.push_back(M0);
    objects.push_back(M1);
    objects.push_back(M2);
    objects.push_back(M3);

	return objects;
}

const std::vector<vec3> g_cubeVertecies{
    { 1.0f, -1.0f, -1.0f },
    { 1.0f, -1.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f },
    { -1.0f, -1.0f, -1.0f },
    { 1.0f, 1.0f, -1.0f },
    {  1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, -1.0f },
};
 
const std::vector<std::array<uint32_t, 3>> g_cubeIndices{
    {1, 3, 0}, 
	{7, 5, 4}, 
	{4, 1, 0},
	{5, 2, 1}, 
	{2, 7, 3}, 
	{0, 7, 4}, 
	{1, 2, 3}, 
	{7, 6, 5}, 
	{4, 5, 1}, 
	{5, 6, 2}, 
	{2, 6, 7}, 
	{0, 3, 7}
};

const std::vector<vec3> g_cubeColors{
	glm::vec3(0, 0, 0),
	glm::vec3(0, 0, 1),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 1, 1),
	glm::vec3(1, 0, 0),
	glm::vec3(1, 0, 1),
	glm::vec3(1, 1, 0),
	glm::vec3(1, 1, 1),
};

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

	const float zNear = 0.1f;
	const float zFar = 100.0f;
	vec3 cameraPos{0.0f, 3.75f, 6.5f};
	vec3 cameraTarget{0.0f, 0.0f, 0.0f};
	vec3 cameraUp{0.0f, 1.0f, 0.0f};	
	auto view = lookAt(cameraPos, cameraTarget, cameraUp);
	auto proj = perspective(glm::radians(60.0f), static_cast<float>(VIEWPORT.x) / static_cast<float>(VIEWPORT.y), zNear, zFar);

	auto transforms = generateRandomTransforms();
	
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

		for (const auto& transform : transforms) {		
			VertexShaderUniforms unif = {proj * view * transform};
			rasterTriangleIndexed(VIEWPORT, g_cubeVertecies, g_cubeColors, g_cubeIndices, unif, vertexShader, fragmentShader, depthBuffer.data(), colorBuffer.data());
		}
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
