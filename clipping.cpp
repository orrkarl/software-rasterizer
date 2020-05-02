#include "clipping.h"

#include "converters.h"

template <typename T>
T apply(T v0, T d1, T d2, vec2 coeffs) {
	return v0 + d1 * coeffs.x + d2 * coeffs.y;
}

size_t clipAgainstPlane(std::array<vec2, 9>& verteciesOut, const std::array<vec2, 9>& verteciesIn, size_t vertexNumIn, float dist0, float dist1, float dist2) {
	size_t vertexNumOut = 0;
	auto currentAIndex = vertexNumIn - 1;

	float currentAValue = apply(dist0, dist1, dist2, verteciesIn[currentAIndex]);
	for (auto currentBIndex = 0u; currentBIndex < vertexNumIn; ++currentBIndex) {
		auto currentBValue = apply(dist0, dist1, dist2, verteciesIn[currentBIndex]);
		if (currentBValue * currentAValue < 0.0f) {
			auto interpB = currentAValue / (currentAValue - currentBValue);
			auto interpA = 1.0f - interpB;
			verteciesOut[vertexNumOut].x = verteciesIn[currentAIndex].x * interpA + verteciesIn[currentBIndex].x * interpB;
			verteciesOut[vertexNumOut].y = verteciesIn[currentAIndex].y * interpA + verteciesIn[currentBIndex].y * interpB;
			vertexNumOut++;
		}
		if (currentBValue >= 0.0f) {
			verteciesOut[vertexNumOut] = verteciesIn[currentBIndex];
			vertexNumOut++;
		}

		currentAValue = currentBValue;
		currentAIndex = currentBIndex;
	}

	return vertexNumOut;
}

bool needClipAxis(const vec4& v, size_t index) {
	return v.w < std::abs(v[index]);
}

bool needClipX(const Triangle& tri) {
	return needClipAxis(tri.v0, 0) || needClipAxis(tri.v1, 0) || needClipAxis(tri.v2, 0); 
}

bool needClipY(const Triangle& tri) {
	return needClipAxis(tri.v0, 1) || needClipAxis(tri.v1, 1) || needClipAxis(tri.v2, 1); 
}

bool needClipZ(const Triangle& tri) {
	return needClipAxis(tri.v0, 2) || needClipAxis(tri.v1, 2) || needClipAxis(tri.v2, 2); 
}

size_t clip(const Triangle& tri, std::array<vec2, 9>& coeffs) {
	auto d1 = tri.v1 - tri.v0;
	auto d2 = tri.v2 - tri.v0;	
	size_t vertexNum = 3;

	coeffs[0] = {0.0, 0.0};
	coeffs[1] = {1.0, 0.0};
	coeffs[2] = {0.0, 1.0};
	
	if (needClipX(tri)) {
		std::array<vec2, 9> temp;
		vertexNum = clipAgainstPlane(temp, coeffs, vertexNum, tri.v0.w + tri.v0.x, d1.w + d1.x, d2.w + d2.x);
		vertexNum = clipAgainstPlane(coeffs, temp, vertexNum, tri.v0.w - tri.v0.x, d1.w - d1.x, d2.w - d2.x);
	}
	if (needClipY(tri)) {
		std::array<vec2, 9> temp;
		vertexNum = clipAgainstPlane(temp, coeffs, vertexNum, tri.v0.w + tri.v0.y, d1.w + d1.y, d2.w + d2.y);
		vertexNum = clipAgainstPlane(coeffs, temp, vertexNum, tri.v0.w - tri.v0.y, d1.w - d1.y, d2.w - d2.y);
	}
	if (needClipZ(tri)) {
		std::array<vec2, 9> temp;
		vertexNum = clipAgainstPlane(temp, coeffs, vertexNum, tri.v0.w + tri.v0.z, d1.w + d1.z, d2.w + d2.z);
		vertexNum = clipAgainstPlane(coeffs, temp, vertexNum, tri.v0.w - tri.v0.z, d1.w - d1.z, d2.w - d2.z);
	}

	return vertexNum;
}
