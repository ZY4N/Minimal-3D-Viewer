#include <graphics/renderers/point_cloud_renderer.hpp>


void point_cloud_renderer::render(
	const std::span<point_cloud_instance> point_clouds,
	const glm::mat4& proj_matrix,
	const glm::mat4& view_matrix
) {
	m_point_shader->bind();
	m_point_shader->set<"proj_mat">(proj_matrix);
	m_point_shader->set<"view_mat">(view_matrix);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SMOOTH);

	for (auto& point_cloud : point_clouds) {
		m_point_shader->bind();
		m_point_shader->set<"model_mat">(point_cloud.transform);

		glBindVertexArray(point_cloud.vba);

		for (auto& attribute : point_cloud.attributes) {
			attribute.pre_render(*m_point_shader);
		}

		for (ztu::isize i = 0; i < point_cloud.num_points; i += ztu::u16_max) {
			const auto elements_left = ztu::isize(point_cloud.num_points) - i;
			glDrawArrays(
				GL_POINTS,
				static_cast<ztu::i32>(i),
				static_cast<ztu::u16>(std::min(ztu::isize(ztu::u16_max), elements_left))
			);
		}

		for (auto& attribute : point_cloud.attributes) {
			attribute.post_render(*m_point_shader);
		}

		glActiveTexture(0);
		glBindVertexArray(0);
	}
}
