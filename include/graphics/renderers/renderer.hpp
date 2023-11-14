#pragma once

#include <vector>
#include <span>
#include <glm/mat4x4.hpp>
#include "graphics/shader.hpp"
#include "graphics/renderables/mesh_instance.hpp"


template<typename T, typename Renderable>
concept renderer = requires(
	T renderer,
	std::span<Renderable> items,
	const glm::mat4& proj_matrix,
	const glm::mat4& view_matrix
) {
	renderer.render(items, proj_matrix, view_matrix);
};
