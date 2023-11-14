#include "graphics/renderers/mesh_point_renderer.hpp"


void mesh_point_renderer::render(
	const std::span<mesh_instance> meshes,
	const glm::mat4& proj_matrix, const glm::mat4& view_matrix
) {
	m_point_shader->bind();
	m_point_shader->set<"proj_mat">(proj_matrix);
	m_point_shader->set<"view_mat">(view_matrix);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SMOOTH);

	for (auto& mesh : meshes) {
		m_point_shader->bind();
		m_point_shader->set<"model_mat">(mesh.transform);

		glBindVertexArray(mesh.vba);

		for (auto& attribute : mesh.attributes) {
			attribute.pre_render(*m_point_shader);
		}

		glDrawElements(GL_POINTS, mesh.num_indices, GL_UNSIGNED_INT, 0);

		for (auto& attribute : mesh.attributes) {
			attribute.post_render(*m_point_shader);
		}

		glActiveTexture(0);
		glBindVertexArray(0);
	}
}
