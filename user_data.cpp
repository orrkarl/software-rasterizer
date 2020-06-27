#include "user_data.h"

#include "converters.h"
#include "rasterizer.h"
#include "util.h"

using glm::perspective;
using glm::radians;

float frac(float x) {
	float tmp = x - static_cast<i64>(x);
	return tmp >= 0.0f ? tmp : 1.0f - tmp;
}

struct FixedColorShader : MiniFragmentShader<vec3, FixedColorShader> {
	vec4 shade(vec3 data) {
		return {data, 1.0f};
	}
};

struct Texture2DSamplerShader : MiniFragmentShader<vec2, Texture2DSamplerShader> {
	Texture2DSamplerShader(u8vec4* tex, u32 w, u32 h) : texture(tex), width(w), height(h) {
	}

	vec4 shade(vec2 data) {
		u16 s = frac(data.s) * width - 0.5f;
		u16 t = frac(data.t) * height - 0.5f;
		u16 idx = t * width + s;
		return vec4(texture[s * width + t]) * (1.0f / 255);
	}

	u8vec4* texture;
	u32 width;
	u32 height;
};

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
vec3 g_cameraPos(0, -8.5, -5);
vec3 g_cameraTarget(20, 5, 1);
vec3 g_cameraUp(0, 1, 0);	

void init(const uvec2& viewport) {
	float nearPlane = 0.125f;
	float farPlane = 5000.f;
	
	g_proj = glm::perspective(
		glm::radians(60.f), 
		static_cast<float>(viewport.x) / static_cast<float>(viewport.y), 
		nearPlane, 
		farPlane);
}

void periodic(GLFWwindow* window, const uvec2& viewport, float* depthBuffer, Color* colorBuffer) {
	constexpr float ANGLE = M_PI / 15;

	g_view = lookAt(g_cameraPos, g_cameraTarget, g_cameraUp);
	g_view = glm::rotate(g_view, glm::radians(-30.f), glm::vec3(0, 1, 0));

	for (const auto& transform : g_transforms) {		
		auto mvp = g_proj * g_view * transform;
		rasterTriangleIndexed<FixedColorShader>(
			viewport, 
			g_cubeVertecies, 
			g_cubeColors, 
			g_cubeIndices, 
			mvp, 
			FixedColorShader(),
			depthBuffer, 
			colorBuffer);
	}
}

