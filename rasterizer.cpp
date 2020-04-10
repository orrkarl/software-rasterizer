#include "rasterizer.h"

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const VertexShaderUniforms& unif, VertexShader vs, FragmentShader fs, float* depthBuffer, Color* colorBuffer) {
	std::vector<VertexShaderOutput> transformed(vertecies.size());

	for (size_t i = 0; i < vertecies.size(); ++i) {
		VertexShaderInput in{&unif, vertecies[i], colors[indices[i][0] % colors.size()]};
		vs(in, transformed[i]);	
	}

	for (size_t triangleIndex = 0; triangleIndex < indices.size(); ++triangleIndex) {
		auto v0Clip = transformed[indices[triangleIndex][0]].gl_Position;
		auto v1Clip = transformed[indices[triangleIndex][1]].gl_Position;
		auto v2Clip = transformed[indices[triangleIndex][2]].gl_Position;

		auto v0 = rasterFromNDC(v0Clip, viewport);
		auto v1 = rasterFromNDC(v1Clip, viewport);
		auto v2 = rasterFromNDC(v2Clip, viewport);

		mat3 vertexMatrix{
			{ v0.x, v1.x, v2.x },
			{ v0.y, v1.y, v2.y },
			{ v0.w, v1.w, v2.w }
		};

		if (determinant(vertexMatrix) >= 0.0f) {
			// degenerate and back-facing triangles are ignored
			continue; 
		}

		auto edgeMatrix = inverse(vertexMatrix);

		auto e0 = edgeMatrix[0];
		auto e1 = edgeMatrix[1];
		auto e2 = edgeMatrix[2];
		auto c = edgeMatrix * vec3{1.0f, 1.0f, 1.0f};
		auto interpolateZ = edgeMatrix * vec3{v0Clip.z, v1Clip.z, v2Clip.z};

		auto edges = transpose(mat3{e0, e1, e2});	
		for (auto y = 0; y < viewport.y; ++y) {
			for (auto x = 0; x < viewport.x; ++x) {
				vec3 sample{x + 0.5f, y + 0.5f, 1.0f};
				auto insides = edges * sample;

				if (all(greaterThanEqual(insides, vec3(0.0f)))) {
					auto oneOverW = dot(c, sample);
					auto w = 1 / oneOverW;
					auto zOverW = dot(interpolateZ, sample);
					auto z = zOverW * w;
					auto bufferIdx = (viewport.y - 1 - y) * viewport.x + x;

					if (z <= depthBuffer[bufferIdx]) {
						depthBuffer[bufferIdx] = z;

						auto barysRaw = insides * w;
						vec3 barys{barysRaw.x, barysRaw.y, 1 - barysRaw.x - barysRaw.y};
						auto c0 = transformed[indices[triangleIndex][0]].custom.color;
						auto c1 = transformed[indices[triangleIndex][1]].custom.color;
						auto c2 = transformed[indices[triangleIndex][2]].custom.color;
						auto interpColor = c0 * barys.x + c1 * barys.y + c2 * barys.z;
						VertexShaderCustomOutput newCustom{interpColor};
						FragmentShaderInput fsInput{newCustom, vec4(sample.x, sample.y, z, oneOverW)};
						vec4 color;
						fragmentShader(fsInput, color);
						colorBuffer[bufferIdx] = mkColor(color);
					}
				}
			}
		}
	}
}
