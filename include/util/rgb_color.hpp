#pragma once

#include <util/uix.hpp>
#include <random>
#include <glm/vec3.hpp>

using rgb_color = glm::vec<3, float, glm::packed_highp>;

namespace rgb_colors {

[[maybe_unused]] static rgb_color random() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<> dist(0, 1);
	rgb_color color{
		 dist(gen),  dist(gen),  dist(gen)
	};
	glm::normalize(color);
	return color;
}

[[maybe_unused]] static rgb_color invert(const rgb_color& c) {
	return { 1.0f - c[0], 1.0f - c[1], 1.0f - c[2] };
}

[[maybe_unused]] static constexpr rgb_color
	black{ 0, 0, 0 },
	white{ 1, 1, 1 },
	red{ 1, 0, 0 },
	yellow{ 1, 1, 0 },
	green{ 0, 1, 0 },
	turquoise{ 0, 1, 1 },
	blue{ 0, 0, 1 },
	pink{ 1, 0, 1 };

}
