#pragma once

#include "predef.h"

struct VertexShaderUniforms {
};

struct VertexShaderInput {
	const VertexShaderUniforms* uniforms;
	vec3 vertex;
	vec3 color;	
};

struct VertexShaderCustomOutput {
	vec4 color;	
};

struct VertexShaderOutput {
	vec4 gl_Position;
	VertexShaderCustomOutput custom;
};

struct FragmentShaderInput {
	VertexShaderCustomOutput vs;
	vec4 gl_FragCoord;
};
using FragmentShaderOutput = vec4;

using VertexShader = std::function<void(const VertexShaderInput&, VertexShaderOutput&)>;
using FragmentShader = std::function<void(const FragmentShaderInput&, FragmentShaderOutput&)>;

void vertexShader(const VertexShaderInput& in, VertexShaderOutput& out); 

void fragmentShader(const FragmentShaderInput& in, FragmentShaderOutput& out); 

