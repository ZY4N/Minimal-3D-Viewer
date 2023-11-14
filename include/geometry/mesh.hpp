#pragma once

#include <vector>
#include <memory>
#include <glm/mat4x4.hpp>
#include "util/uix.hpp"
#include "geometry/vertex_component.hpp"
#include "graphics/renderables/mesh_instance.hpp"
#include "geometry/aabb.hpp"
#include "geometry/material.hpp"


template<vertex_component... Cs>
class mesh {
public:
	using vertex = std::tuple<vertex_components::position, Cs...>;
	using vertex_t = std::tuple<vertex_components::position::type, typename Cs::type...>;

public:
	mesh(
		std::vector<typename mesh<Cs...>::vertex_t>&& vertexBuffer,
		std::vector<ztu::u32>&& indexBuffer
	);

	mesh(
		const std::vector<typename mesh<Cs...>::vertex_t>& vertexBuffer,
		const std::vector<ztu::u32>& indexBuffer
	);

	mesh(const mesh<Cs...>& other);

	mesh(mesh<Cs...>&& other) noexcept;

	mesh<Cs...>& operator=(const mesh<Cs...>& other);

	mesh<Cs...>& operator=(mesh<Cs...>&& other) noexcept;

	~mesh();

	void init_vao();

	[[nodiscard]] const std::vector<vertex_t>& vertex_buffer() const;

	[[nodiscard]] std::vector<vertex_t>& vertex_buffer();

	[[nodiscard]] const std::vector<ztu::u32>& index_buffer() const;

	[[nodiscard]] std::vector<ztu::u32>& index_buffer();

	[[nodiscard]] std::optional<mesh_instance> create_instance(
		const glm::mat4x4& model_matrix = glm::identity<glm::mat4x4>()
	) const;

	[[nodiscard]] aabb calc_bounding_box() const;

protected:
	std::vector<vertex_t> m_vertices;
	std::vector<ztu::u32> m_indices;

	ztu::u32 m_vertex_buffer_id{ 0 };
	ztu::u32 m_index_buffer_id{ 0 };
	ztu::u32 m_vao_id{ 0 };

public:
	std::weak_ptr<material> m_material;
};

#define INCLUDE_MESH_IMPLEMENTATION
#include "geometry/mesh.ipp"


#undef INCLUDE_MESH_IMPLEMENTATION
