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
		interpolateZ = edgeMatrix * vec3{v0Clip.z, v1Clip.z, v2Clip.z};

		vec3 e0 = edgeMatrix[0];
		vec3 e1 = edgeMatrix[1];
		vec3 e2 = edgeMatrix[2];
		edges = transpose(mat3{e0, e1, e2});
	}

	float area;
	vec3 interpolateW;
	mat3 edges;	 
	vec3 interpolateZ;
};

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const VertexShaderUniforms& unif, VertexShader vs, FragmentShader fs, float* depthBuffer, Color* colorBuffer) {
	std::vector<VertexShaderOutput> transformed(vertecies.size());

	for (size_t i = 0; i < vertecies.size(); ++i) {
		VertexShaderInput in{&unif, vertecies[i], colors[i]};
		vs(in, transformed[i]);	
	}

	for (size_t triangleIndex = 0; triangleIndex < indices.size(); ++triangleIndex) {
		Triangle raw{
			transformed[indices[triangleIndex][0]].gl_Position,
			transformed[indices[triangleIndex][1]].gl_Position,
			transformed[indices[triangleIndex][2]].gl_Position
		};
		std::array<vec2, 9> coeffs;
		const auto v0 = raw.v0;
		const auto d1 = raw.v1 - raw.v0;
		const auto d2 = raw.v2 - raw.v0;
		auto vertexCount = clip(raw, coeffs);
		
		auto v0Clip = v0 + d1 * coeffs[0].x + d2 * coeffs[0].y;
		auto v1Clip = v0 + d1 * coeffs[1].x + d2 * coeffs[1].y;
		for (auto i = 2; i < vertexCount; ++i) {
			auto v1Clip = v0 + d1 * coeffs[i - 1].x + d2 * coeffs[i - 1].y;
			auto v2Clip = v0 + d1 * coeffs[i].x + d2 * coeffs[i].y;

			TriangleRecord record(v0Clip, v1Clip, v2Clip, viewport);
			
			if (record.area >= 0.0f) {
				// degenerate and back-facing triangles are ignored
				continue; 
			}

			const auto origC0 = transformed[indices[triangleIndex][0]].custom.color;
			const auto origC1 = transformed[indices[triangleIndex][1]].custom.color;
			const auto origC2 = transformed[indices[triangleIndex][2]].custom.color;
			auto clippedC0 = origC0 + (origC1 - origC0) * coeffs[0].x + (origC2 - origC0) * coeffs[0].y; 
			auto clippedC1 = origC0 + (origC1 - origC0) * coeffs[i - 1].x + (origC2 - origC0) * coeffs[i - 1].y; 
			auto clippedC2 = origC0 + (origC1 - origC0) * coeffs[i].x + (origC2 - origC0) * coeffs[i].y; 
			for (auto y = 0; y < viewport.y; ++y) {
				for (auto x = 0; x < viewport.x; ++x) {
					vec3 sample{x + 0.5f, y + 0.5f, 1.0f};
					vec3 insides = record.edges * sample;

					if (all(greaterThanEqual(insides, vec3(0)))) {
						auto oneOverW = record.interpolateW.x * sample.x + record.interpolateW.y * sample.y + record.interpolateW.z * sample.z;
						auto zOverW = record.interpolateZ.x * sample.x + record.interpolateZ.y * sample.y + record.interpolateZ.z * sample.z;
						auto z = zOverW / static_cast<float>(oneOverW);

						auto bufferIdx = (viewport.y - 1 - y) * viewport.x + x;

						if (z <= depthBuffer[bufferIdx]) {
							depthBuffer[bufferIdx] = z;

							vec2 barysRaw{
								insides.x / static_cast<float>(oneOverW),
								insides.y / static_cast<float>(oneOverW)
							};
							vec3 barys{barysRaw.x, barysRaw.y, 1 - barysRaw.x - barysRaw.y};
							auto interpColor = clippedC0 * barys.x + clippedC1 * barys.y + clippedC2 * barys.z;
							VertexShaderCustomOutput newCustom{interpColor};
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
