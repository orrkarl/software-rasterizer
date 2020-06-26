#include "RasterUtil.h"

u16vec2 rasterFromNDC(const vec4& clip, const vec2& viewport) {
	auto shiftedNdcX = 1 + clip.x / clip.w;
	auto shiftedNdcY = 1 - clip.y / clip.w;
	return {shiftedNdcX * viewport.x / 2, shiftedNdcY * viewport.y / 2};
}

float normalizeDepth(const vec4& v) {
	return (1 + v.z / v.w) * 0.5f;
}

TriangleRecord::TriangleRecord(const vec4& v0Clip, const vec4& v1Clip, const vec4& v2Clip, const uvec2& viewport) {
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

