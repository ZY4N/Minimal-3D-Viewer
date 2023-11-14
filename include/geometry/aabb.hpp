#pragma once

#include <glm/vec3.hpp>
#include <span>


struct aabb {
	glm::vec3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
	glm::vec3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	[[nodiscard]] glm::vec3 size() const {
		return max - min;
	}

	void join(const aabb& other) {
		min = glm::min(min, other.min);
		max = glm::max(max, other.max);
	}

	static inline std::pair<glm::vec3, glm::vec3> calc_min_max(std::span<const glm::vec3> vertices) {
		auto min = glm::vec3{ FLT_MAX, FLT_MAX, FLT_MAX };
		auto max = glm::vec3{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (const auto& vertex : vertices) {
			min = glm::min(min, vertex);
			max = glm::max(max, vertex);
		}
		return { min, max };
	}

	void transform(const glm::mat4x4& matrix) {
		const auto vertices = {
			glm::vec3{ matrix * glm::vec4{ min.x, min.y, min.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ min.x, min.y, max.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ min.x, max.y, min.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ min.x, max.y, max.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ max.x, min.y, min.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ max.x, min.y, max.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ max.x, max.y, min.z, 1 } },
			glm::vec3{ matrix * glm::vec4{ max.x, max.y, max.z, 1 } }
		};
		std::tie(min, max) = calc_min_max(vertices);
	}
};
