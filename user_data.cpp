#include "user_data.h"

#include "converters.h"
#include "rasterizer.h"
#include "util.h"

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

std::vector<mat4> g_transforms;
mat4 g_view;
mat4 g_proj;
vec3 g_cameraPos{0.0f, 3.75f, 6.5f};
vec3 g_cameraTarget{0.0f, 0.0f, 0.0f};
vec3 g_cameraUp{0.0f, 1.0f, 0.0f};	

void init(const uvec2& viewport) {
	const float zNear = 0.1f;
	const float zFar = 100.0f;

	g_proj = perspective(glm::radians(60.0f), static_cast<float>(viewport.x) / static_cast<float>(viewport.y), zNear, zFar);
	g_transforms = generateRandomTransforms();
}

void periodic(GLFWwindow* window, const uvec2& viewport, float* depthBuffer, Color* colorBuffer) {
	constexpr float ANGLE = M_PI / 15;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		g_cameraPos = vec3(glm::eulerAngleY(ANGLE) * vec4(g_cameraPos, 1.0f));
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		g_cameraPos = vec3(glm::eulerAngleY(-ANGLE) * vec4(g_cameraPos, 1.0f));	
	}
	g_view = lookAt(g_cameraPos, g_cameraTarget, g_cameraUp);
	for (const auto& transform : g_transforms) {		
		auto mvp = g_proj * g_view * transform;
		rasterTriangleIndexed(viewport, g_cubeVertecies, g_cubeColors, g_cubeIndices, mvp, depthBuffer, colorBuffer);
	}
}

