#pragma once

#include "predef.h"
#include "user_data.h"
#include "converters.h"

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const VertexShaderUniforms& unif, VertexShader vs, FragmentShader fs, float* depthBuffer, Color* colorBuffer); 

