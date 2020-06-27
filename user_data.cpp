#include "user_data.h"

#include <map>
#include <memory>
#include <stb/stb_image.h>
#include <stdexcept>
#include <tinyobjloader/tiny_obj_loader.h>

#include "converters.h"
#include "rasterizer.h"
#include "util.h"

using glm::perspective;
using glm::radians;

float frac(float x) {
	float tmp = x - static_cast<i64>(x);
	return tmp >= 0.0f ? tmp : 1.0f - tmp;
}

struct Mesh {
	uint32_t baseIndex;
	uint32_t indexCount;
	std::string texName;
};

struct SortedVertex {
	i32 positionIdx;
	i32 texCoordIdx;

	bool operator<(const SortedVertex& other) const {
		if (positionIdx < other.positionIdx) {
			return true;
		}

		return texCoordIdx < other.texCoordIdx; 
	}
};

struct Texture {
	std::unique_ptr<stbi_uc> buffer;
	i32 width;
	i32 height;
	i32 numChannels;
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
		u32 idx = (t * texture.width + s) * texture.numChannels;
		auto bufferAtIdx = texture.buffer.get() + idx;
		return vec4(bufferAtIdx[0], bufferAtIdx[1], bufferAtIdx[2], 255.0f) * (1.0f / 255);
	}

	Texture& texture;
};

std::vector<vec3> g_vertecies;
std::vector<vec2> g_texCoords;
std::vector<u32> g_indices;
std::vector<Mesh> g_meshes;
std::map<std::string, std::unique_ptr<Texture>> g_textures;

mat4 g_view;
mat4 g_proj;
vec3 g_cameraPos(0, -8.5, -5);
vec3 g_cameraTarget(20, 5, 1);
vec3 g_cameraUp(0, 1, 0);	

std::map<std::string, std::unique_ptr<Texture>> loadMaterials(const std::vector<tinyobj::material_t>& materials) {
	std::map<std::string, std::unique_ptr<Texture>> ret;
	for (const auto& mat : materials) {
		auto name = mat.diffuse_texname;
		if (name.empty()) {
			std::cerr << "Missing texture file" << std::endl;
			throw std::runtime_error("no tex file");
		}

		if (ret.find(name) == ret.end()) {
			auto actualMaterialPath = "../resources/" + name;
			auto newTex = std::make_unique<Texture>();
			newTex->buffer = std::unique_ptr<stbi_uc>(stbi_load(actualMaterialPath.c_str(), 
				&newTex->width, &newTex->height, &newTex->numChannels, 0));

			if (newTex->buffer == nullptr) {
				std::cerr << "Could not load material file: " << actualMaterialPath << std::endl;
				throw std::runtime_error("material file load failed");
			}

			ret.insert({name, std::move(newTex)});
		}
	}
}

void loadScene(
	const std::string& sceneFileName, 
	std::vector<vec3>& vertecies, std::vector<vec2>& texCoords, std::vector<u32>& indices, std::vector<Mesh>& meshes,
	std::map<std::string, std::unique_ptr<Texture>>& textures) {
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

	textures = loadMaterials(materials);	

	std::map<SortedVertex, uint32_t> indexedVertecies;
	for (const auto& shape : shapes) {
		auto meshBaseIdx = uint32_t(indices.size());	
		for (const auto& index : shape.mesh.indices) {
			auto vertIdx = index.vertex_index;
			auto texCoordIdx = index.texcoord_index;
			if (vertIdx == -1 || texCoordIdx == -1) {
				throw std::runtime_error("missing vertex data");
			}

			SortedVertex vert{vertIdx, texCoordIdx};	
			auto foundVert = indexedVertecies.find(vert);

			if (foundVert != indexedVertecies.end()) {
				indices.push_back(foundVert->second);
			} else {
				auto newIdx = indices.size();
				indexedVertecies[vert] = newIdx;
				indices.push_back(newIdx);	

				vertecies.push_back(vec3(
					attribs.vertices[3 * vertIdx + 0], 
					attribs.vertices[3 * vertIdx + 1], 
					attribs.vertices[3 * vertIdx + 2]));
				texCoords.push_back(vec2(
					attribs.texcoords[2 * texCoordIdx + 0],
					1.0f - attribs.texcoords[2 * texCoordIdx + 1]));				
			}
		}

		Mesh newMesh{meshBaseIdx, u32(shape.mesh.indices.size()), materials[shape.mesh.material_ids[0]].diffuse_texname};
		meshes.push_back(newMesh); 
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

	loadScene("sponza.obj", g_vertecies, g_texCoords, g_indices, g_meshes, g_textures);
}

void periodic(GLFWwindow* window, const uvec2& viewport, float* depthBuffer, Color* colorBuffer) {
	constexpr float ANGLE = M_PI / 15;

	g_view = lookAt(g_cameraPos, g_cameraTarget, g_cameraUp);
	g_view = glm::rotate(g_view, glm::radians(-30.f), glm::vec3(0, 1, 0));

	auto mvp = g_proj * g_view;

}

