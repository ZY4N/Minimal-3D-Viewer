#pragma once

#include "renderer.hpp"
#include "graphics/renderables/point_cloud_instance.hpp"
#include <graphics/shaders.hpp>


class point_cloud_renderer {
public:
	using point_shader_t = shaders::points;

public:
	explicit point_cloud_renderer(point_shader_t* n_point_shader) :
		m_point_shader{ n_point_shader } {
	};

	void render(
		std::span<point_cloud_instance> point_clouds,
		const glm::mat4& proj_matrix,
		const glm::mat4& view_matrix
	);

private:
	point_shader_t* m_point_shader;
};

static_assert(renderer<point_cloud_renderer, point_cloud_instance>);