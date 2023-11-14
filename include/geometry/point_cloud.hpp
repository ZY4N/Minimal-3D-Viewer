#pragma once

#include <memory>
#include <vector>
#include <glm/mat4x4.hpp>
#include "util/uix.hpp"
#include "geometry/aabb.hpp"
#include "geometry/vertex_component.hpp"
#include "graphics/renderables/point_cloud_instance.hpp"


template<vertex_component... Cs>
class point_cloud {
public:
	using vertex = std::tuple<vertex_components::position, Cs...>;
	using vertex_t = std::tuple<vertex_components::position::type, typename Cs::type...>;

public:
	explicit point_cloud(
		std::vector<typename point_cloud<Cs...>::vertex_t>&& points
	);

	explicit point_cloud(
		const std::vector<typename point_cloud<Cs...>::vertex_t>& points
	);

	point_cloud(const point_cloud<Cs...>& other);

	point_cloud(point_cloud<Cs...>&& other) noexcept;

	point_cloud<Cs...>& operator=(const point_cloud<Cs...>& other);

	point_cloud<Cs...>& operator=(point_cloud<Cs...>&& other) noexcept;

	~point_cloud();

	void init_vao();

	[[nodiscard]] const std::vector<vertex_t>& points() const;

	[[nodiscard]] std::optional<point_cloud_instance> create_instance(
		const glm::mat4x4& model_matrix = glm::identity<glm::mat4x4>()
	) const;

	[[nodiscard]] aabb calc_bounding_box() const;


protected:
	std::vector<vertex_t> m_points;

	GLuint m_vertex_buffer_id{ 0 };
	GLuint m_vao_id{ 0 };
};


#define INCLUDE_POINT_CLOUD_IMPLEMENTATION
#include "geometry/point_cloud.ipp"


#undef INCLUDE_POINT_CLOUD_IMPLEMENTATION
