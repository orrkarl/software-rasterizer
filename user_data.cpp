#include "user_data.h"

#include <tinyobjloader/tiny_obj_loader.h>
#include <map>
#include <memory>

#include "converters.h"
#include "rasterizer.h"
#include "util.h"

using glm::perspective;
using glm::radians;

float frac(float x) {
	float tmp = x - static_cast<i64>(x);
	return tmp >= 0.0f ? tmp : 1.0f - tmp;
}

struct Texture {
	Texture(u8vec4* tex, u32 w, u32 h) : buffer(tex), width(w), height(h) {
	}

	std::unique_ptr<u8vec4> buffer;
	u32 width;
	u32 height;
};

struct FixedColorShader : MiniFragmentShader<vec3, FixedColorShader> {
	vec4 shade(vec3 data) {
		return {data, 1.0f};
	}
};

struct Texture2DSamplerShader : MiniFragmentShader<vec2, Texture2DSamplerShader> {
	Texture2DSamplerShader(Texture& tex) : texture(tex) {
	}

	vec4 shade(vec2 data) {
		u16 s = frac(data.s) * texture.width - 0.5f;
		u16 t = frac(data.t) * texture.height - 0.5f;
		u16 idx = t * texture.width + s;
		auto buffer = texture.buffer.get();
		return vec4(buffer[s * texture.width + t]) * (1.0f / 255);
	}

	Texture& texture;
};

std::vector<vec3> g_vertecies;
std::vector<vec2> g_texCoords;
std::vector<u32> g_indices;
std::map<std::string, std::unique_ptr<Texture>> g_textures;

mat4 g_view;
mat4 g_proj;
vec3 g_cameraPos(0, -8.5, -5);
vec3 g_cameraTarget(20, 5, 1);
vec3 g_cameraUp(0, 1, 0);	

void loadScene(
	const std::string& sceneFileName, 
	std::vector<vec3>& vertecies, std::vector<vec2>& texCoords, std::vector<u32>& indices) {
	tinyobj::attrib_t attribs;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	auto actualFilePath = "../resources/" + sceneFileName;
	auto status = tinyobj::LoadObj(
		&attribs, &shapes, &materials, 
		&warn, &err, 
		actualFilePath.c_str(), "../resources",
		true, true);

	if (!warn.empty()) {
		std::cerr << "TinyObj loaded with warnings:" << std::endl;
		std::cerr << warn << std::endl;
	}
	if (!status) {
		std::cerr << "TinyObj failed to load " << actualFilePath << ":" << std::endl;
		std::cerr << err << std::endl;
		throw std::runtime_error("TinyObj failed to load");
	}

	
}

void init(const uvec2& viewport) {
	float nearPlane = 0.125f;
	float farPlane = 5000.f;
	
	g_proj = glm::perspective(
		glm::radians(60.f), 
		static_cast<float>(viewport.x) / static_cast<float>(viewport.y), 
		nearPlane, 
		farPlane);

	loadScene("sponza.obj", g_vertecies, g_texCoords, g_indices);
}

void periodic(GLFWwindow* window, const uvec2& viewport, float* depthBuffer, Color* colorBuffer) {
	constexpr float ANGLE = M_PI / 15;

	g_view = lookAt(g_cameraPos, g_cameraTarget, g_cameraUp);
	g_view = glm::rotate(g_view, glm::radians(-30.f), glm::vec3(0, 1, 0));

	auto mvp = g_proj * g_view;

}

