#include "graphics/renderers/mesh_line_renderer.hpp"
#include <util/logger.hpp>


void mesh_line_renderer::render(
	const std::span<mesh_instance> meshes,
	const glm::mat4& proj_matrix,
	const glm::mat4& view_matrix
) {
	m_line_shader->bind();
	m_line_shader->set<"proj_mat">(proj_matrix);
	m_line_shader->set<"view_mat">(view_matrix);


	for (auto& mesh : meshes) {
		m_line_shader->bind();
		m_line_shader->set<"model_mat">(mesh.transform);

		glBindVertexArray(mesh.vba);

		for (auto& attribute : mesh.attributes) {
			attribute.pre_render(*m_line_shader);
		}

		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);

		for (auto& attribute : mesh.attributes) {
			attribute.post_render(*m_line_shader);
		}

		glBindVertexArray(0);
	}
}