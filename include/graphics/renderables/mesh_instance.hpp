#pragma once

#include <vector>
#include <util/uix.hpp>
#include "graphics/renderable_attributes.hpp"
#include "graphics/dynamic_renderable_attribute.hpp"


using mesh_attributes = dynamic_renderable_attribute<
	renderable_attributes::color,
	renderable_attributes::texture,
	renderable_attributes::point_size
>;

struct mesh_instance {
	ztu::u32 vba;
	size_t num_indices;
	glm::mat4x4 transform;
	std::vector<mesh_attributes> attributes;
};
