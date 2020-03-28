#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <thread>
#include <vector>

#include "converters.h"

using namespace glm;

std::vector<mat4> generateRandomTransforms() {
	std::vector<mat4> objects;

	const mat4 identity(1.f);
 
    auto M0 = translate(identity, glm::vec3(0, 0, 2.f));
    M0 = rotate(M0, glm::radians(45.f), glm::vec3(0, 1, 0));
    objects.push_back(M0);
 
    auto M1 = translate(identity, glm::vec3(-3.75f, 0, 0));
    M1 = rotate(M1, glm::radians(30.f), glm::vec3(1, 0, 0));
    objects.push_back(M1);
 
    auto M2 = translate(identity, glm::vec3(3.75f, 0, 0));
    M2 = rotate(M2, glm::radians(60.f), glm::vec3(0, 1, 0));
    objects.push_back(M2);
 
    auto M3 = glm::translate(identity, glm::vec3(0, 0, -2.f));
    M3 = rotate(M3, glm::radians(90.f), glm::vec3(0, 0, 1));
    objects.push_back(M3);

	return objects;
}

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec4>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, size_t triangleIndex, float* depthBuffer, Color* colorBuffer) {
	auto v0 = rasterFromNDC(vertecies[indices[triangleIndex][0]], viewport);
	auto v1 = rasterFromNDC(vertecies[indices[triangleIndex][1]], viewport);
	auto v2 = rasterFromNDC(vertecies[indices[triangleIndex][2]], viewport);

	mat3 vertexMatrix{
		{ v0.x, v1.x, v2.x },
		{ v0.y, v1.y, v2.y },
		{ v0.w, v1.w, v2.w }
	};

	if (determinant(vertexMatrix) >= 0.0f) {
		// degenerate and back-facing triangles are ignored
		return; 
	}

	auto edgeMatrix = inverse(vertexMatrix);

	auto e0 = edgeMatrix[0];
	auto e1 = edgeMatrix[1];
	auto e2 = edgeMatrix[2];
	auto c = edgeMatrix * vec3{1.0f, 1.0f, 1.0f};

	auto edges = transpose(mat3{e0, e1, e2});	
	
	for (auto y = 0; y < viewport.y; ++y) {
		for (auto x = 0; x < viewport.x; ++x) {
			vec3 sample{x + 0.5f, y + 0.5f, 1.0f};
			auto insides = edges * sample;

			if (all(greaterThanEqual(insides, vec3(0.0f)))) {
				auto oneOverW = dot(c, sample);
				if (oneOverW >= depthBuffer[y * viewport.x + x]) {
					depthBuffer[y * viewport.x + x] = oneOverW;
					colorBuffer[y * viewport.x + x] = mkColor(colors[triangleIndex % colors.size()]);
				}
			}
		}
	}
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
	glm::vec3(0, 0, 1),
	glm::vec3(0, 1, 0),
	glm::vec3(0, 1, 1),
	glm::vec3(1, 1, 1),
	glm::vec3(1, 0, 1),
	glm::vec3(1, 1, 0)
};

void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

int main() {
	const uvec2 VIEWPORT{1366, 768};
	const float DEPTH_BUFFER_CLEAR = 0.0f;
	const Color COLOR_BUFFER_CLEAR = {0, 0, 0, 1};
	std::vector<float> depthBuffer(VIEWPORT.x * VIEWPORT.y, DEPTH_BUFFER_CLEAR);
	std::vector<Color> colorBuffer(VIEWPORT.x * VIEWPORT.y, COLOR_BUFFER_CLEAR);
	std::chrono::milliseconds frameWait(30);

	if (!glfwInit()) {
		std::cerr << "could not init GLFW" << std::endl;
		return EXIT_FAILURE;
	}
	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	auto window = glfwCreateWindow(VIEWPORT.x, VIEWPORT.y, "software rasterization", nullptr, nullptr);
	if (!window) {
		std::cerr << "could not create window" << std::endl;
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glViewport(0, 0, VIEWPORT.x, VIEWPORT.y);
	glClear(GL_COLOR_BUFFER_BIT);

	const float zNear = 0.1f;
	const float zFar = 100.0f;
	vec3 cameraPos{0.0f, 3.75f, 6.5f};
	vec3 cameraTarget{0.0f, 0.0f, 0.0f};
	vec3 cameraUp{0.0f, 1.0f, 0.0f};	
	auto view = lookAt(cameraPos, cameraTarget, cameraUp);
	auto proj = perspective(glm::radians(60.0f), static_cast<float>(VIEWPORT.x) / static_cast<float>(VIEWPORT.y), zNear, zFar);

	auto transforms = generateRandomTransforms();
	
	auto lastFrameLogTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window)) {
		auto start = std::chrono::high_resolution_clock::now();
		std::fill(depthBuffer.begin(), depthBuffer.end(), DEPTH_BUFFER_CLEAR);
		std::fill(colorBuffer.begin(), colorBuffer.end(), COLOR_BUFFER_CLEAR);

		for (const auto& transform : transforms) {		
			const auto MVP = proj * view * transform;
			std::vector<vec4> transformedVertecies(g_cubeVertecies.size());
			std::transform(g_cubeVertecies.cbegin(), g_cubeVertecies.cend(), transformedVertecies.begin(), [MVP](const auto& vec){return MVP * vec4(vec, 1.0f);});

			for (size_t triangleIndex = 0; triangleIndex < g_cubeIndices.size(); ++triangleIndex) {
				rasterTriangleIndexed(VIEWPORT, transformedVertecies, g_cubeColors, g_cubeIndices, triangleIndex, depthBuffer.data(), colorBuffer.data());
			}
		}
		glDrawPixels(VIEWPORT.x, VIEWPORT.y, GL_RGBA, GL_UNSIGNED_BYTE, colorBuffer.data());

		glfwSwapBuffers(window);
		glfwPollEvents();
		auto end = std::chrono::high_resolution_clock::now();
		if ((end - lastFrameLogTime) > std::chrono::seconds(1)) {
			std::cout << "Frame rendering took " << ((end - start).count()) / 1000000 << "ms" << std::endl;
			lastFrameLogTime = end;
		}
		std::this_thread::sleep_for(frameWait);		
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
