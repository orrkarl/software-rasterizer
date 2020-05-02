#include "user_data.h"

#include "converters.h"
#include "shaders.h"
#include "rasterizer.h"
#include "util.h"

const std::vector<vec3> g_triangleVertecies{
    { -2.0f, -0.5f, 0.5f },
    { 2.0f, -0.5f, 0.5f },
    { 0.0f, 0.5f, 0.5f },
};
 
const std::vector<std::array<uint32_t, 3>> g_triangleIndices{
    {0, 1, 2}, 
};

const std::vector<vec3> g_triangleColors{
	glm::vec3(0, 0, 1),
	glm::vec3(0, 1, 0),
	glm::vec3(1, 0, 0)
};

void init(const uvec2& viewport) {
}

void periodic(const uvec2& viewport, float* depthBuffer, Color* colorBuffer) {
	VertexShaderUniforms unif;
	rasterTriangleIndexed(viewport, g_triangleVertecies, g_triangleColors, g_triangleIndices, unif, vertexShader, fragmentShader, depthBuffer, colorBuffer);
}

