#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <system_error>

#include "util/uix.hpp"
#include "geometry/mesh.hpp"
#include "graphics/renderable_attributes.hpp"


namespace mesh_loader_error {

enum class codes {
	ok = 0,
	obj_cannot_open_file,
	obj_malformed_vertex,
	obj_malformed_texture_coordinate,
	obj_malformed_normal,
	obj_malformed_face,
	obj_face_index_out_of_range,
	obj_unknown_line_begin,
	mtl_cannot_open_file,
	mtl_cannot_open_texture,
	mtl_malformed_color,
	mtl_malformed_color_alpha,
	mlt_unknown_line_begin
};

} // namespace mesh_loader_error

namespace mesh_loader {

template<vertex_component... Cs>
std::error_code load_from_obj(
	const std::filesystem::path& filename,
	std::vector<mesh<Cs...>>& mesh,
	std::unordered_map<std::string, std::shared_ptr<material>>& materials,
	bool pedantic = false
);

mesh_loader_error::codes parse_mtl(
	const std::filesystem::path& filename,
	std::unordered_map<std::string, std::shared_ptr<material>>& materials,
	bool pedantic = false
);

} // namespace mesh_loader

#define INCLUDE_MESH_LOADER_IMPLEMENTATION
#include "geometry/mesh_loader.ipp"


#undef INCLUDE_MESH_LOADER_IMPLEMENTATION
