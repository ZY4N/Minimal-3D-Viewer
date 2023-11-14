#pragma once

#include "renderer.hpp"
#include "graphics/shaders.hpp"


class mesh_line_renderer {
public:
	using line_shader_t = shaders::mesh_lines;

public:
	explicit mesh_line_renderer(line_shader_t* n_line_shader) :
		m_line_shader{ n_line_shader } {
	};

	void render(
		std::span<mesh_instance> meshes,
		const glm::mat4& proj_matrix,
		const glm::mat4& view_matrix
	);

private:
	line_shader_t* m_line_shader;
};

static_assert(renderer<mesh_line_renderer, mesh_instance>);
