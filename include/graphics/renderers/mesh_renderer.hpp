#pragma once

#include "renderer.hpp"
#include "graphics/shaders.hpp"


class mesh_renderer {
public:
	using mesh_shader_t = shaders::meshes;

public:
	explicit mesh_renderer(mesh_shader_t* n_mesh_shader) :
		m_mesh_shader{ n_mesh_shader } {
	};

	void render(
		std::span<mesh_instance> meshes,
		const glm::mat4& proj_matrix,
		const glm::mat4& view_matrix
	);

private:
	mesh_shader_t* m_mesh_shader;
};

static_assert(renderer<mesh_renderer, mesh_instance>);
