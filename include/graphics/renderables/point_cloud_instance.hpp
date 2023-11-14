#pragma once

#include <vector>
#include "util/uix.hpp"
#include "graphics/renderable_attributes.hpp"
#include "graphics/dynamic_renderable_attribute.hpp"


using point_cloud_attributes = dynamic_renderable_attribute<
	renderable_attributes::color,
	renderable_attributes::point_size
>;

struct point_cloud_instance {
	ztu::u32 vba;
	ztu::isize num_points;
	glm::mat4x4 transform;
	std::vector<point_cloud_attributes> attributes;
};