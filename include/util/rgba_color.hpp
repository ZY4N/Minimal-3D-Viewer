#pragma once

#include <util/uix.hpp>
#include <random>
#include <glm/vec4.hpp>

using rgba_color = glm::vec<4, float, glm::packed_highp>;

namespace rgba_colors {

[[maybe_unused]]  static rgba_color random() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<> dist(0, 1);
	rgba_color color{
		 dist(gen),  dist(gen),  dist(gen), 1.0f
	};
	glm::normalize(color);
	return color;
}

[[maybe_unused]] static rgba_color invert(const rgba_color& c) {
	return {
		1.0f - c[0],
		1.0f - c[1],
		1.0f - c[2],
		c[3]
	};
}

[[maybe_unused]]  static constexpr rgba_color
	black{ 0, 0, 0, 1 },
	white{ 1, 1, 1, 1 },
	red{ 1, 0, 0, 1 },
	yellow{ 1, 1, 0, 1 },
	green{ 0, 1, 0, 1 },
	turquoise{ 0, 1, 1, 1 },
	blue{ 0, 0, 1, 1 },
	pink{ 1, 0, 1, 1 };

}
