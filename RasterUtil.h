#pragma once

#include "predef.h"

#include "TypeUtil.h"

template <typename T, glm::qualifier Q>
mat<3, 3, T, Q> adjoint(mat<3, 3, T, Q> const& m);

u16vec2 rasterFromNDC(const vec4& clip, const vec2& viewport); 

float normalizeDepth(const vec4& v); 

struct TriangleRecord {
	TriangleRecord(const vec4& v0Clip, const vec4& v1Clip, const vec4& v2Clip, const uvec2& viewport); 

	i32 area;
	imat3 edges;	 
	vec3 interpolatedZ;
	vec3 oneOverW;
};

#include "RasterUtil.inl"

