#include "rasterizer.h"

#include "clipping.h"

template <typename T, glm::qualifier Q>
mat<3, 3, T, Q> adjoint(mat<3, 3, T, Q> const& m)
{
	mat<3, 3, T, Q> ret;

	ret[0][0] = + (m[1][1] * m[2][2] - m[2][1] * m[1][2]);
	ret[1][0] = - (m[1][0] * m[2][2] - m[2][0] * m[1][2]);
	ret[2][0] = + (m[1][0] * m[2][1] - m[2][0] * m[1][1]);
	ret[0][1] = - (m[0][1] * m[2][2] - m[2][1] * m[0][2]);
	ret[1][1] = + (m[0][0] * m[2][2] - m[2][0] * m[0][2]);
	ret[2][1] = - (m[0][0] * m[2][1] - m[2][0] * m[0][1]);
	ret[0][2] = + (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
	ret[1][2] = - (m[0][0] * m[1][2] - m[1][0] * m[0][2]);
	ret[2][2] = + (m[0][0] * m[1][1] - m[1][0] * m[0][1]);

	return ret;
}

using lmat3 = mat<3, 3, int64_t, glm::highp>;
using lvec3 = vec<3, int64_t>;

constexpr auto SUBPIXEL = 1 << 4;

u16vec2 rasterFromNDC(const vec4& clip, const vec2& viewport) {
	auto shiftedNdcX = 1 + clip.x / clip.w;
	auto shiftedNdcY = 1 - clip.y / clip.w;
	return {shiftedNdcX * viewport.x / 2, shiftedNdcY * viewport.y / 2};
}

float normalizeDepth(const vec4& v) {
	return (1 + v.z / v.w) * 0.5f;
}

struct TriangleRecord {
	TriangleRecord(const vec4& v0Clip, const vec4& v1Clip, const vec4& v2Clip, const uvec2& viewport) {
		auto v0 = rasterFromNDC(v0Clip, viewport);
		auto v1 = rasterFromNDC(v1Clip, viewport);
		auto v2 = rasterFromNDC(v2Clip, viewport);

		lmat3 vertexMatrix{
			{ v0.x, v1.x, v2.x },
			{ v0.y, v1.y, v2.y },
			{ 1, 1, 1 }
		};
		area = determinant(vertexMatrix);

		if (area >= 0) { 
			return; 
		}

		edges = transpose(adjoint(vertexMatrix));
		interpolatedZ = {normalizeDepth(v0Clip), normalizeDepth(v1Clip), normalizeDepth(v2Clip)}; 
		oneOverW = 1.0f / vec3{ v0Clip.w, v1Clip.w, v2Clip.w };
	}

	i32 area;
	lmat3 edges;	 
	vec3 interpolatedZ;
	vec3 oneOverW;
};

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const mat4& mvp, float* depthBuffer, Color* colorBuffer) {
	std::vector<vec4> transformedVertecies(vertecies.size());
	std::vector<vec4> transformedColors(vertecies.size());

	for (size_t i = 0; i < vertecies.size(); ++i) {
		transformedVertecies[i] = mvp * vec4(vertecies[i], 1.0f);
		transformedColors[i] = vec4(colors[i], 1.0f);		
	}

	for (size_t triangleIndex = 0; triangleIndex < indices.size(); ++triangleIndex) {
		Triangle raw{
			transformedVertecies[indices[triangleIndex][0]],
			transformedVertecies[indices[triangleIndex][1]],
			transformedVertecies[indices[triangleIndex][2]]
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
			
			if (record.area >= 0) {
				// degenerate and back-facing triangles are ignored
				continue; 
			}

			const auto origC0 = transformedColors[indices[triangleIndex][0]];
			const auto origC1 = transformedColors[indices[triangleIndex][1]];
			const auto origC2 = transformedColors[indices[triangleIndex][2]];
			const glm::mat3x4 clippedColor{
				origC0 + (origC1 - origC0) * coeffs[0].x + (origC2 - origC0) * coeffs[0].y,
				origC0 + (origC1 - origC0) * coeffs[i - 1].x + (origC2 - origC0) * coeffs[i - 1].y,
				origC0 + (origC1 - origC0) * coeffs[i].x + (origC2 - origC0) * coeffs[i].y
			};

			for (auto y = 0; y < viewport.y; ++y) {
				for (auto x = 0; x < viewport.x; ++x) {
					auto sample = SUBPIXEL * ivec3(2 * x + 1, 2 * y + 1, 2);
					auto insides = record.edges * sample;
					i64 areaSign = sign(record.area);

					if (all(greaterThanEqual(areaSign * insides, lvec3(0)))) {
						auto bufferIdx = (viewport.y - 1 - y) * viewport.x + x;
						auto insidesNormalize = 1 / static_cast<float>(static_cast<i64>(record.area) * SUBPIXEL * 2);

						vec2 barysRaw(insides.x * insidesNormalize, insides.y * insidesNormalize);
						vec3 barys(barysRaw.x, barysRaw.y, 1 - barysRaw.x - barysRaw.y);

						auto z = dot(barys, record.interpolatedZ);

						if (z <= depthBuffer[bufferIdx]) {
							depthBuffer[bufferIdx] = z;

							auto interpColor = clippedColor * (record.oneOverW * barys) / dot(record.oneOverW, barys); 
							vec4 color = interpColor;
							colorBuffer[bufferIdx] = mkColor(color);
						}
					}
				}
			}
		}
	}
}
