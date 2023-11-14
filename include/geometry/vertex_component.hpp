#pragma once

#include <util/uix.hpp>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <util/rgb_color.hpp>


template<typename T>
concept vertex_component = (
	sizeof(typename T::component_type) * T::count == sizeof(typename T::type)
);

namespace vertex_component_internal {

template<auto Count, typename Component, ztu::usize UUID>
struct base_vertex_component {
	using type = glm::vec<Count, Component, glm::packed_highp>;
	static constexpr auto count = Count;
	using component_type = Component;
	static constexpr auto uuid = UUID;
};

} // namespace vertex_component_internal

namespace vertex_components {

using position = vertex_component_internal::base_vertex_component<3, float, 0>;
using tex_coord = vertex_component_internal::base_vertex_component<2, float, 1>;
using normal = vertex_component_internal::base_vertex_component<3, float, 2>;
using color = vertex_component_internal::base_vertex_component<3, float, 3>;
using reflectance = vertex_component_internal::base_vertex_component<1, float, 4>;

} // namespace vertex_components
