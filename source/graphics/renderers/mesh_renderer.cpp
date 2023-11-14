#include "graphics/renderers/mesh_renderer.hpp"

void mesh_renderer::render(
	const std::span<mesh_instance> meshes,
	const glm::mat4& proj_matrix,
	const glm::mat4& view_matrix
) {
	m_mesh_shader->bind();
	m_mesh_shader->set<"proj_mat">(proj_matrix);
	m_mesh_shader->set<"view_mat">(view_matrix);

	for (auto& mesh : meshes) {
		m_mesh_shader->bind();
		glActiveTexture(GL_TEXTURE0);
		
		m_mesh_shader->set<"model_mat">(mesh.transform);

		glBindVertexArray(mesh.vba);

		for (auto& attribute : mesh.attributes) {
			attribute.pre_render(*m_mesh_shader);
		}

		glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);

		for (auto& attribute : mesh.attributes) {
			attribute.post_render(*m_mesh_shader);
		}

		glActiveTexture(0);
		glBindVertexArray(0);
	}
}