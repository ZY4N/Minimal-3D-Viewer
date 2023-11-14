#pragma once

#include "renderer.hpp"
#include "graphics/shaders.hpp"


struct mesh_point_renderer {
public:
	using mesh_point_shader_t = shaders::mesh_points;

public:
	explicit mesh_point_renderer(mesh_point_shader_t* n_point_shader) :
		m_point_shader{ n_point_shader } {
	};

	void render(
		std::span<mesh_instance> renderables,
		const glm::mat4& proj_matrix, const glm::mat4& view_matrix
	);

private:
	mesh_point_shader_t* m_point_shader;
};
