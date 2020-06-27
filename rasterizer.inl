#include "clipping.h"
#include "RasterUtil.h"

namespace detail {

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

		imat3 vertexMatrix{
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
	imat3 edges;	 
	vec3 interpolatedZ;
	vec3 oneOverW;
};

}

template <typename FragmentShader>
void rasterTriangleIndexed(
	const uvec2& viewport, 
	const std::vector<vec3>& vertecies, 
	const std::vector<typename FragmentShader::Input>& colors, 
	const std::vector<std::array<uint32_t, 3>>& indices, 
	const mat4& mvp, 
	FragmentShader fs,
	float* depthBuffer, 
	Color* colorBuffer) {
	std::vector<vec4> transformedVertecies(vertecies.size());

	for (size_t i = 0; i < vertecies.size(); ++i) {
		transformedVertecies[i] = mvp * vec4(vertecies[i], 1.0f);
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

			detail::TriangleRecord record(v0Clip, v1Clip, v2Clip, viewport);
			
			if (record.area >= 0) {
				// degenerate and back-facing triangles are ignored
				continue; 
			}

			const auto origC0 = colors[indices[triangleIndex][0]];
			const auto origC1 = colors[indices[triangleIndex][1]];
			const auto origC2 = colors[indices[triangleIndex][2]];
			const glm::mat<3, FragmentShader::InputDimension, float> clippedColor{
				origC0 + (origC1 - origC0) * coeffs[0].x + (origC2 - origC0) * coeffs[0].y,
				origC0 + (origC1 - origC0) * coeffs[i - 1].x + (origC2 - origC0) * coeffs[i - 1].y,
				origC0 + (origC1 - origC0) * coeffs[i].x + (origC2 - origC0) * coeffs[i].y
			};

			for (auto y = 0; y < viewport.y; ++y) {
				for (auto x = 0; x < viewport.x; ++x) {
					auto sample = SUBPIXEL * ivec3(2 * x + 1, 2 * y + 1, 2);
					auto insides = record.edges * sample;

					if (all(lessThanEqual(insides, ivec3(0)))) {
						auto bufferIdx = (viewport.y - 1 - y) * viewport.x + x;
						auto insidesNormalize = 1 / static_cast<float>(static_cast<i64>(record.area) * SUBPIXEL * 2);

						vec2 barysRaw(insides.x * insidesNormalize, insides.y * insidesNormalize);
						vec3 barys(barysRaw.x, barysRaw.y, 1 - barysRaw.x - barysRaw.y);

						auto z = dot(barys, record.interpolatedZ);

						if (z <= depthBuffer[bufferIdx]) {
							depthBuffer[bufferIdx] = z;

							auto interpolatedData = clippedColor * (record.oneOverW * barys) / dot(record.oneOverW, barys);
							auto resultColor = fs.shade(interpolatedData); 
							colorBuffer[bufferIdx] = mkColor(resultColor);
						}
					}
				}
			}
		}
	}
}

