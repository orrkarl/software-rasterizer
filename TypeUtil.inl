template <>
struct DetermineDimension<float> {
	static constexpr uint32_t Value = 1;
};

template <>
struct DetermineDimension<vec2> {
	static constexpr uint32_t Value = 2;
};

template <>
struct DetermineDimension<vec3> {
	static constexpr uint32_t Value = 3;
};

template <>
struct DetermineDimension<vec4> {
	static constexpr uint32_t Value = 4;
};

