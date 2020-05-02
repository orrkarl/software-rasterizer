#include "rasterizer.h"

#include "clipping.h"

using lmat3 = mat<3, 3, int64_t, glm::highp>;
using lvec3 = vec<3, int64_t>;

constexpr uint32_t BITS = 1 << 24;

int32_t normalize(float f) {
	return static_cast<int32_t>(f * BITS);
}

ivec3 normalize(const vec3& v) {
	return ivec3{normalize(v.x), normalize(v.y), normalize(v.z)};
}

struct TriangleRecord {
	TriangleRecord(const vec4& v0Clip, const vec4& v1Clip, const vec4& v2Clip, const uvec2& viewport) {
		vec4 v0 = rasterFromNDC(v0Clip, viewport);
		vec4 v1 = rasterFromNDC(v1Clip, viewport);
		vec4 v2 = rasterFromNDC(v2Clip, viewport);

		mat3 vertexMatrix{
			{ v0.x, v1.x, v2.x },
			{ v0.y, v1.y, v2.y },
			{ v0.w, v1.w, v2.w }
		};
		area = determinant(vertexMatrix);

		if (area >= 0.0f) { 
			return; 
		}

		mat3 edgeMatrix = inverse(vertexMatrix);

		interpolateW = edgeMatrix * vec3{1.0f, 1.0f, 1.0f};
		auto interpolateZ = edgeMatrix * vec3{v0Clip.z, v1Clip.z, v2Clip.z};

		vec3 e0 = edgeMatrix[0];
		vec3 e1 = edgeMatrix[1];
		vec3 e2 = edgeMatrix[2];

		auto e0I = normalize(e0);
		auto e1I = normalize(e1);
		auto e2I = normalize(e2);
		edgesI = transpose(lmat3{e0I, e1I, e2I});

		interpolateWI = normalize(interpolateW);
		interpolateZI = normalize(interpolateZ);
	}

	float area;
	vec3 interpolateW;
	lmat3 edgesI;	 
	ivec3 interpolateWI;
	ivec3 interpolateZI;
};

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const VertexShaderUniforms& unif, VertexShader vs, FragmentShader fs, float* depthBuffer, Color* colorBuffer) {
	std::vector<VertexShaderOutput> transformed(vertecies.size());

	for (size_t i = 0; i < vertecies.size(); ++i) {
		VertexShaderInput in{&unif, vertecies[i], colors[indices[i][0] % colors.size()]};
		vs(in, transformed[i]);	
	}

	for (size_t triangleIndex = 0; triangleIndex < indices.size(); ++triangleIndex) {
		Triangle raw{
			transformed[indices[triangleIndex][0]].gl_Position,
			transformed[indices[triangleIndex][1]].gl_Position,
			transformed[indices[triangleIndex][2]].gl_Position
		};
		auto triangles = clip(raw);

		for (const auto& current : triangles) {
			auto v0Clip = current.v0;
			auto v1Clip = current.v1;
			auto v2Clip = current.v2;

			TriangleRecord record(v0Clip, v1Clip, v2Clip, viewport);
			
			if (record.area >= 0.0f) {
				// degenerate and back-facing triangles are ignored
				continue; 
			}

			for (auto y = 0; y < viewport.y; ++y) {
				for (auto x = 0; x < viewport.x; ++x) {
					ivec3 sampleI{2 * x + 1, 2 * y + 1, 2};
					lvec3 insidesI = record.edgesI * sampleI;

					if (all(greaterThanEqual(insidesI, lvec3(0)))) {
						auto oneOverWI = record.interpolateWI.x * sampleI.x + record.interpolateWI.y * sampleI.y + record.interpolateWI.z * sampleI.z;
						auto zOverW = record.interpolateZI.x * sampleI.x + record.interpolateZI.y * sampleI.y + record.interpolateZI.z * sampleI.z;
						auto z = zOverW / static_cast<float>(oneOverWI);

						auto bufferIdx = (viewport.y - 1 - y) * viewport.x + x;

						if (z <= depthBuffer[bufferIdx]) {
							depthBuffer[bufferIdx] = z;

							vec3 barysRaw{
								insidesI.x / static_cast<float>(oneOverWI),
								insidesI.y / static_cast<float>(oneOverWI),
								insidesI.z / static_cast<float>(oneOverWI),
							};
							vec3 barys{barysRaw.x, barysRaw.y, 1 - barysRaw.x - barysRaw.y};
							auto c0 = transformed[indices[triangleIndex][0]].custom.color;
							auto c1 = transformed[indices[triangleIndex][1]].custom.color;
							auto c2 = transformed[indices[triangleIndex][2]].custom.color;
							auto interpColor = c0 * barys.x + c1 * barys.y + c2 * barys.z;
							VertexShaderCustomOutput newCustom{interpColor};
							vec3 sample{x + 0.5, y + 0.5f, 1};
							auto w = dot(record.interpolateW, sample);
							FragmentShaderInput fsInput{newCustom, vec4(sample.x, sample.y, z, w)};
							vec4 color;
							fragmentShader(fsInput, color);
							colorBuffer[bufferIdx] = mkColor(color);
						}
					}
				}
			}
		}
	}
}
