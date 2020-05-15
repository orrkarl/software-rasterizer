#pragma once

#include "predef.h"

using Color = glm::u8vec4;

template <int D, class T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& os, const glm::vec<D, T, Q>& vec) {
	os << "<";
	for (auto i = 0u; i < D - 1; ++i) {
		os << vec[i] << ", ";
	}
	os << vec[D - 1];
	return os << ">";
}

template <int N, int M, class T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& os, const glm::mat<M, N, T, Q>& mat) {
	auto matT = glm::transpose(mat);

	os << "mat" << N << "x" << M << "{" << std::endl;
	for (auto i = 0u; i < N; ++i) { 
		os << '\t' << matT[i] << std::endl;
	}

	return os << "}";
}


Color mkColor(const glm::vec4& color); 

Color mkColor(const glm::vec3& color); 

