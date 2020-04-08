#include <algorithm>
#include <array> 
#include <chrono>
#include <cmath>
#include <functional>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

#include "converters.h"
#include "util.h"

using namespace glm;

struct VertexShaderUniforms {
	mat4 mvp;
};

struct VertexShaderInput {
	const VertexShaderUniforms* uniforms;
	vec3 vertex;
	vec3 color;	
};

struct VertexShaderCustomOutput {
	vec4 color;	
};

struct VertexShaderOutput {
	vec4 gl_Position;
	VertexShaderCustomOutput custom;
};

struct FragmentShaderInput {
	VertexShaderCustomOutput vs;
	vec4 gl_FragCoord;
};
using FragmentShaderOutput = vec4;

using VertexShader = std::function<void(const VertexShaderInput&, VertexShaderOutput&)>;
using FragmentShader = std::function<void(const FragmentShaderInput&, FragmentShaderOutput&)>;

void vertexShader(const VertexShaderInput& in, VertexShaderOutput& out) {
	out.gl_Position = in.uniforms->mvp * vec4(in.vertex, 1.0f);
	out.custom.color = vec4(in.color, 1.0f);		
}

void fragmentShader(const FragmentShaderInput& in, FragmentShaderOutput& out) {
	out = in.vs.color;
}

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

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const VertexShaderUniforms& unif, VertexShader vs, FragmentShader fs, float* depthBuffer, Color* colorBuffer) {
	std::vector<VertexShaderOutput> transformed(vertecies.size());

	for (size_t i = 0; i < vertecies.size(); ++i) {
		VertexShaderInput in{&unif, vertecies[i], colors[indices[i][0] % colors.size()]};
		vs(in, transformed[i]);	
	}

	for (size_t triangleIndex = 0; triangleIndex < indices.size(); ++triangleIndex) {
		auto v0Clip = transformed[indices[triangleIndex][0]].gl_Position;
		auto v1Clip = transformed[indices[triangleIndex][1]].gl_Position;
		auto v2Clip = transformed[indices[triangleIndex][2]].gl_Position;

		auto v0 = rasterFromNDC(v0Clip, viewport);
		auto v1 = rasterFromNDC(v1Clip, viewport);
		auto v2 = rasterFromNDC(v2Clip, viewport);

		mat3 vertexMatrix{
			{ v0.x, v1.x, v2.x },
			{ v0.y, v1.y, v2.y },
			{ v0.w, v1.w, v2.w }
		};

		if (determinant(vertexMatrix) >= 0.0f) {
			// degenerate and back-facing triangles are ignored
			continue; 
		}

		auto edgeMatrix = inverse(vertexMatrix);

		auto e0 = edgeMatrix[0];
		auto e1 = edgeMatrix[1];
		auto e2 = edgeMatrix[2];
		auto c = edgeMatrix * vec3{1.0f, 1.0f, 1.0f};
		auto interpolateZ = edgeMatrix * vec3{v0Clip.z, v1Clip.z, v2Clip.z};

		auto edges = transpose(mat3{e0, e1, e2});	
		for (auto y = 0; y < viewport.y; ++y) {
			for (auto x = 0; x < viewport.x; ++x) {
				vec3 sample{x + 0.5f, y + 0.5f, 1.0f};
				auto insides = edges * sample;

				if (all(greaterThanEqual(insides, vec3(0.0f)))) {
					auto oneOverW = dot(c, sample);
					auto w = 1 / oneOverW;
					auto zOverW = dot(interpolateZ, sample);
					auto z = zOverW * w;
					auto bufferIdx = (viewport.y - 1 - y) * viewport.x + x;

					if (z <= depthBuffer[bufferIdx]) {
						depthBuffer[bufferIdx] = z;

						FragmentShaderInput fsInput{transformed[indices[triangleIndex][0] % transformed.size()].custom, vec4(sample.x, sample.y, z, oneOverW)};
						vec4 color;
						fragmentShader(fsInput, color);
						colorBuffer[bufferIdx] = mkColor(color);
					}
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

int main() {
	const uvec2 VIEWPORT{1280, 720};
	const float DEPTH_BUFFER_CLEAR = std::numeric_limits<float>::max();
	const Color COLOR_BUFFER_CLEAR = {0, 0, 0, 1};
	const bool RENDER_ONCE = false;
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
		if (RENDER_ONCE && rendered) {
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
