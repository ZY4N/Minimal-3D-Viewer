#pragma once

#include <vector>
#include <system_error>
#include "geometry/point_cloud.hpp"
#include "geometry/vertex_component.hpp"


using basic_point_cloud = point_cloud<>;
using reflectance_point_cloud = point_cloud<vertex_components::reflectance>;

namespace point_cloud_loader {

using basic_vertex = basic_point_cloud::vertex_t;
using reflectance_vertex = reflectance_point_cloud::vertex_t;

[[nodiscard]] inline std::error_code load_from_3dtk_directory(
	const std::filesystem::path& directory,
	std::vector<basic_point_cloud>& basic_point_cloud,
	std::vector<reflectance_point_cloud>& reflectance_point_cloud
);

[[nodiscard]] inline std::error_code analyze_3dtk_file(
	const std::filesystem::path& filename,
	ztu::u32& num_floats,
	std::chars_format& format
);


template<bool Reflectance, bool Hex>
[[nodiscard]] inline std::error_code load_from_3dtk_file(
	const std::filesystem::path& base_filename,
	std::vector<basic_vertex>& basic_points,
	std::vector<reflectance_vertex>& points
);


[[nodiscard]] inline std::error_code write_v1_c3d_file(
	const std::filesystem::path& filename,
	/*const glm::vec3& position,
	const glm::vec3& direction,*/
	const std::vector<reflectance_vertex>& points
);

[[nodiscard]] inline std::error_code load_v1_c3d_file(
	const std::filesystem::path& filename,
	std::vector<reflectance_vertex>& points
);

} // namespace point_cloud_loader


#define INCLUDE_POINT_CLOUD_LOADER_IMPLEMENTATION
#include <geometry/point_cloud_loader.ipp>


#undef INCLUDE_POINT_CLOUD_LOADER_IMPLEMENTATION
