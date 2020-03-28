#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <raylib.h>
#include <vector>

#include "converters.h"

using namespace glm;

const std::vector<vec3> g_cubeVertices{
    { 1.0f, -1.0f, -1.0f },
    { 1.0f, -1.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f },
    { -1.0f, -1.0f, -1.0f },
    { 1.0f, 1.0f, -1.0f },
    {  1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, -1.0f },
};
 
const std::vector<uint32_t> cubeIndices{
    1,3,0, 7,5,4, 4,1,0, 5,2,1, 2,7,3, 0,7,4, 1,2,3, 7,6,5, 4,5,1, 5,6,2, 2,6,7, 0,3,7
};

std::vector<mat4> createRandomTransforms() {
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

mat3 rasterTriangleSetup(const vec3& v0, const vec3& v1, const vec3& v2) {
	mat3 vertexMatrix{
		{ v0.x, v1.x, v2.x },
		{ v0.y, v1.y, v2.y },
		{ v0.z, v1.z, v2.z }
	};

	// Compute the inverse of vertex matrix to use it for setting up edge functions
	auto edgeMatrix = inverse(vertexMatrix);

	// Calculate edge functions based on the vertex matrix
	auto e0 = edgeMatrix[0];
	auto e1 = edgeMatrix[1];
	auto e2 = edgeMatrix[2];

	return transpose(mat3{e0, e1, e2});	
}

void rasterTriangle(const uvec2& viewport, const vec3& v0, const vec3& v1, const vec3& v2) {
	auto edges = rasterTriangleSetup(viewportFromNDC(v0, viewport), viewportFromNDC(v1, viewport), viewportFromNDC(v2, viewport));

	for (auto y = 0; y < viewport.y; ++y) {
		for (auto x = 0; x < viewport.x; ++x) {
			vec3 sample{x + 0.5f, y + 0.5f, 1.0f};
			auto insides = edges * sample;

			if (all(greaterThanEqual(insides, vec3(0.0f)))) {
				DrawPixel(x, viewport.y - 1 - y, mkColor({1.0f, 0.0f, 0.0f}));
			}
		}
	}
}

int main() {
	const uvec2 VIEWPORT{640, 480};

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(VIEWPORT.x, VIEWPORT.y, "software rasterization");
	SetTargetFPS(60);                           

	const float zNear = 0.1f;
	const float zFar = 100.0f;
	vec3 cameraPos{0.0f, 3.75f, 6.5f};
	vec3 cameraTarget{0.0f, 0.0f, 0.0f};
	vec3 cameraUp{0.0f, 1.0f, 0.0f};	
	auto view = lookAt(cameraPos, cameraTarget, cameraUp);
	auto proj = perspective(glm::radians(45.0f), static_cast<float>(VIEWPORT.x) / static_cast<float>(VIEWPORT.y), zNear, zFar);

	vec3 v0{-0.5f, -0.5f, 1.0f};
	vec3 v1{0.0f, 0.5f, 1.0f};
	vec3 v2{0.5f, -0.5f, 1.0f};

	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(BLACK);
		rasterTriangle(VIEWPORT, v0, v1, v2);

		EndDrawing();
	}

	CloseWindow();
}
