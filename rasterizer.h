#pragma once

#include "converters.h"
#include "predef.h"
#include "TypeUtil.h"
#include "user_data.h"

constexpr auto SUBPIXEL = 1 << 4;

template <typename T, typename Impl>
struct MiniFragmentShader {
	using Input = T;
	static constexpr auto InputDimension = DetermineDimensionValue<T>;
	
	vec4 shade(T data) {
		return static_cast<Impl*>(this)->shade(data);
	}
};

template <typename FragmentShader>
void rasterTriangleIndexed(
	const uvec2& viewport, 
	const std::vector<vec3>& vertecies, 
	const std::vector<typename FragmentShader::Input>& colors, 
	const std::vector<std::array<uint32_t, 3>>& indices, 
	const mat4& mvp, 
	FragmentShader fs,
	float* depthBuffer, 
	Color* colorBuffer);

#include "rasterizer.inl"
