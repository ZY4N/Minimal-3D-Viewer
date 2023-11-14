#ifndef INCLUDE_POINT_CLOUD_IMPLEMENTATION
#error Never include this file directly include 'point_cloud.hpp'
#endif


#include <SFML/OpenGL.hpp>
#include "util/logger.hpp"
#include "graphics/to_gl_type.hpp"


template<vertex_component... Cs>
point_cloud<Cs...>::point_cloud(
	const std::vector<typename point_cloud<Cs...>::vertex_t>& n_points
) : m_points{ n_points } {
}

template<vertex_component... Cs>
point_cloud<Cs...>::point_cloud(
	std::vector<typename point_cloud<Cs...>::vertex_t>&& n_points
) : m_points{ std::move(n_points) } {
}

template<vertex_component... Cs>
point_cloud<Cs...>::~point_cloud() {
	if (m_vertex_buffer_id) {
		glDeleteBuffers(1, &m_vertex_buffer_id);
	}
}

template<vertex_component... Cs>
point_cloud<Cs...>::point_cloud(const point_cloud<Cs...>& other) :
	m_points{ other.m_points } {
}

template<vertex_component... Cs>
point_cloud<Cs...>::point_cloud(point_cloud<Cs...>&& other) noexcept:
	m_points{ std::move(other.m_points) },
	m_vertex_buffer_id{ other.m_vertex_buffer_id },
	m_vao_id{ other.m_vao_id } {
	other.m_vao_id = 0;
	other.m_vertex_buffer_id = 0;
}

template<vertex_component... Cs>
point_cloud<Cs...>& point_cloud<Cs...>::operator=(const point_cloud<Cs...>& other) {

	if (&other != this) {
		this->~point_cloud();

		m_points = other.m_points;
		m_vao_id = 0;
		m_vertex_buffer_id = 0;
	}

	return *this;
}

template<vertex_component... Cs>
point_cloud<Cs...>& point_cloud<Cs...>::operator=(point_cloud<Cs...>&& other) noexcept {

	if (&other != this) {
		glDeleteBuffers(1, &m_vertex_buffer_id);

		m_points = std::move(other.m_points);
		m_vao_id = other.m_vao_id;
		m_vertex_buffer_id = other.m_vertex_buffer_id;

		other.m_vertex_buffer_id = 0;
		other.m_vao_id = 0;
	}

	return *this;
}

template<vertex_component... Cs>
void point_cloud<Cs...>::init_vao() {

	if (m_vao_id) {
		return;
	}

	glGenVertexArrays(1, &m_vao_id);
	glBindVertexArray(m_vao_id);

	glGenBuffers(1, &m_vertex_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(vertex_t), &m_points[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);

	const auto& first_vertex = m_points.front();

	ztu::for_each::index<std::tuple_size_v<vertex>>(
		[&first_vertex]<auto Index>() {
			const auto offset = static_cast<ztu::usize>(
				reinterpret_cast<const char*>(&std::get<Index>(first_vertex)) -
					reinterpret_cast<const char*>(&first_vertex)
			);
			using component = std::tuple_element_t<Index, vertex>;
			glVertexAttribPointer(
				Index,
				component::count,
				to_gl_type<typename component::component_type>(),
				GL_FALSE,
				sizeof(vertex_t),
				reinterpret_cast<GLvoid*>(offset)
			);
			glEnableVertexAttribArray(Index);
			return false;
		}
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

template<vertex_component... Cs>
std::optional<point_cloud_instance> point_cloud<Cs...>::create_instance(const glm::mat4x4& model_matrix) const {
	if (not m_vao_id) {
		return std::nullopt;
	}

	return point_cloud_instance(m_vao_id, m_points.size(), model_matrix, {});
}

template<vertex_component... Cs>
aabb point_cloud<Cs...>::calc_bounding_box() const {
	aabb box;
	box.min = glm::vec3{ FLT_MAX, FLT_MAX, FLT_MAX };
	box.max = glm::vec3{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (const auto& point : m_points) {
		const auto& position = std::get<0>(point);
		box.min = glm::min(box.min, position);
		box.max = glm::max(box.max, position);
	}
	return box;
}

template<vertex_component... Cs>
const std::vector<typename point_cloud<Cs...>::vertex_t>& point_cloud<Cs...>::points() const {
	return m_points;
}
