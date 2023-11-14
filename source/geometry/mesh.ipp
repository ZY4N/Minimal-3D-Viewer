#ifndef INCLUDE_MESH_IMPLEMENTATION
#error Never include this file directly include 'mesh.hpp'
#endif

#include <SFML/OpenGL.hpp>
#include "graphics/to_gl_type.hpp"


template<vertex_component... Cs>
mesh<Cs...>::mesh(
	const std::vector<typename mesh<Cs...>::vertex_t>& vertexBuffer,
	const std::vector<ztu::u32>& indexBuffer
) : m_vertices{ vertexBuffer }, m_indices{ indexBuffer } {
}


template<vertex_component... Cs>
mesh<Cs...>::mesh(
	std::vector<typename mesh<Cs...>::vertex_t>&& vertexBuffer,
	std::vector<ztu::u32>&& indexBuffer
) : m_vertices{ std::move(vertexBuffer) }, m_indices{ std::move(indexBuffer) } {
}

template<vertex_component... Cs>
mesh<Cs...>::~mesh() {
	if (m_vertex_buffer_id) {
		glDeleteBuffers(1, &m_vertex_buffer_id);
	}
	if (m_index_buffer_id) {
		glDeleteBuffers(1, &m_index_buffer_id);
	}
}

template<vertex_component... Cs>
mesh<Cs...>::mesh(const mesh<Cs...>& other) :
	m_vertices{ other.m_vertices },
	m_indices{ other.m_indices },
	m_material{ other.m_material } {
}

template<vertex_component... Cs>
mesh<Cs...>::mesh(mesh<Cs...>&& other) noexcept:
	m_vertices{ std::move(other.m_vertices) },
	m_indices{ std::move(other.m_indices) },
	m_vertex_buffer_id{ other.m_vertex_buffer_id },
	m_index_buffer_id{ other.m_index_buffer_id },
	m_vao_id{ other.m_vao_id },
	m_material{ other.m_material } {
	other.m_vao_id = 0;
	other.m_vertex_buffer_id = 0;
	other.m_index_buffer_id = 0;
}

template<vertex_component... Cs>
mesh<Cs...>& mesh<Cs...>::operator=(const mesh<Cs...>& other) {
	if (&other != this) {
		glDeleteBuffers(1, &m_vertex_buffer_id);
		glDeleteBuffers(1, &m_index_buffer_id);
		m_vertex_buffer_id = 0;
		m_index_buffer_id = 0;
		m_vao_id = 0;

		m_vertices = other.m_vertices;
		m_indices = other.m_indices;
		m_material = other.m_material;
	}

	return *this;
}

template<vertex_component... Cs>
mesh<Cs...>& mesh<Cs...>::operator=(mesh<Cs...>&& other) noexcept {
	if (&other != this) {
		glDeleteBuffers(1, &m_vertex_buffer_id);
		glDeleteBuffers(1, &m_index_buffer_id);

		m_vertices = std::move(other.m_vertices);
		m_indices = std::move(other.m_indices);

		m_vao_id = other.m_vao_id;
		m_vertex_buffer_id = other.m_vertex_buffer_id;
		m_index_buffer_id = other.m_index_buffer_id;

		m_material = other.m_material;

		other.m_vertex_buffer_id = 0;
		other.m_index_buffer_id = 0;
		other.m_vao_id = 0;
	}

	return *this;
}

template<vertex_component... Cs>
void mesh<Cs...>::init_vao() {
	glGenVertexArrays(1, &m_vao_id);
	glBindVertexArray(m_vao_id);

	glGenBuffers(1, &m_vertex_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(vertex_t), &m_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_index_buffer_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(ztu::u32), &m_indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);

	const auto& first_vertex = m_vertices.front();

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

	if (auto mtl_ptr = m_material.lock()) {
		mtl_ptr->init_attributes();
	}
}

template<vertex_component... Cs>
std::optional<mesh_instance> mesh<Cs...>::create_instance(const glm::mat4x4& model_matrix) const {
	if (not m_vao_id) {
		return std::nullopt;
	}

	std::vector<mesh_attributes> attributes;
	if (auto mtl_ptr = m_material.lock()) {
		if (mtl_ptr->m_color_attribute) {
			attributes.emplace_back(mtl_ptr->m_color_attribute);
		}
		if (mtl_ptr->m_texture_attribute) {
			attributes.emplace_back(mtl_ptr->m_texture_attribute);
		}
	}

	return mesh_instance{ m_vao_id, m_indices.size(), model_matrix, std::move(attributes) };
}

template<vertex_component... Cs>
aabb mesh<Cs...>::calc_bounding_box() const {
	aabb box;
	box.min = glm::vec3{ FLT_MAX, FLT_MAX, FLT_MAX };
	box.max = glm::vec3{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (const auto& vertex : m_vertices) {
		const auto& position = std::get<0>(vertex);
		box.min = glm::min(box.min, position);
		box.max = glm::max(box.max, position);
	}
	return box;
}

template<vertex_component... Cs>
const std::vector<typename mesh<Cs...>::vertex_t>& mesh<Cs...>::vertex_buffer() const {
	return m_vertices;
}

template<vertex_component... Cs>
std::vector<typename mesh<Cs...>::vertex_t>& mesh<Cs...>::vertex_buffer() {
	return m_vertices;
}

template<vertex_component... Cs>
const std::vector<ztu::u32>& mesh<Cs...>::index_buffer() const {
	return m_indices;
}

template<vertex_component... Cs>
std::vector<ztu::u32>& mesh<Cs...>::index_buffer() {
	return m_indices;
}
