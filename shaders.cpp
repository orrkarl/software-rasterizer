#include "shaders.h"

void vertexShader(const VertexShaderInput& in, VertexShaderOutput& out) {
	out.gl_Position = in.uniforms->mvp * vec4(in.vertex, 1.0f);
	out.custom.color = vec4(in.color, 1.0f);		
}

void fragmentShader(const FragmentShaderInput& in, FragmentShaderOutput& out) {
	out = in.vs.color;
}

