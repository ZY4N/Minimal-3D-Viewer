#ifndef INCLUDE_MESH_LOADER_IMPLEMENTATION
#error Never include this file directly include 'mesh_loader.hpp'
#endif

#include <fstream>

namespace mesh_loader_error {

struct category : std::error_category {
	[[nodiscard]] const char* name() const noexcept override {
		return "connector";
	}

	[[nodiscard]] std::string message(int ev) const override {
		switch (static_cast<codes>(ev)) {
			using
			enum codes;
		case obj_cannot_open_file:
			return "Cannot open given obj file";
		case obj_malformed_vertex:
			return "Malformed 'v' statement";
		case obj_malformed_texture_coordinate:
			return "Malformed 'vt' statement";
		case obj_malformed_normal:
			return "Malformed 'vn' statement";
		case obj_malformed_face:
			return "Malformed 'f' statement";
		case obj_face_index_out_of_range:
			return "Face index out of range";
		case obj_unknown_line_begin:
			return "Unknown obj line begin";
		case mtl_cannot_open_file:
			return "Cannot open mtl file";
		case mtl_cannot_open_texture:
			return "Cannot open texture file";
		case mtl_malformed_color:
			return "Malformed 'Kd' statement";
		case mtl_malformed_color_alpha:
			return "malformed 'd' statement";
		case mlt_unknown_line_begin:
			return "Unknown mtl line begin";
		default:
			using namespace std::string_literals;
			return "unrecognized error ("s + std::to_string(ev) + ")";
		}
	}
};

} // namespace mesh_loader_error


inline std::error_category& connector_error_category() {
	static mesh_loader_error::category category;
	return category;
}


namespace mesh_loader_error {

inline std::error_code make_error_code(codes e) {
	return { static_cast<int>(e), connector_error_category() };
}

} // namespace mesh_loader_error


template<vertex_component... Cs>
struct vertex_id {
	std::array<ztu::u32, std::tuple_size_v<typename mesh<Cs...>::vertex_t>> indices{ };

	friend auto operator<=>(const vertex_id&, const vertex_id&) = default;
};

template<vertex_component... Cs>
struct indexed_vertex_id {
	vertex_id<vertex_components::position, Cs...> id;
	ztu::u32 bufferIndex{ ztu::u32_max };

	indexed_vertex_id(std::span<const ztu::u32, std::tuple_size_v<typename mesh<Cs...>::vertex_t>> indices) {
		std::copy(indices.begin(), indices.end(), id.indices.begin());
	}

	friend auto operator<=>(const indexed_vertex_id& a, const indexed_vertex_id& b) {
		return a.id <=> b.id;
	}

	bool operator==(const indexed_vertex_id& other) const noexcept {
		return other.id == id;
	}
};


template<typename F>
struct prefixed_parser {
	const std::string_view prefix;
	F parse;
};

template<class... Fs>
bool parse_line(std::string_view line, prefixed_parser<Fs>&& ... parsers) {
	return ztu::for_each::argument(
		[&](auto&& parser) {
			if (line.starts_with(parser.prefix)) {
				parser.parse(line.substr(parser.prefix.length()));
				return true;
			}
			return false;
		}, parsers...
	);
}


template<vertex_component... Cs>
std::error_code mesh_loader::load_from_obj(
	const std::filesystem::path& filename,
	std::vector<mesh<Cs...>>& destination,
	std::unordered_map<std::string, std::shared_ptr<material>>& materials,
	bool pedantic
) {
	using
	enum mesh_loader_error::codes;
	using mesh_loader_error::make_error_code;

	auto in = std::ifstream{ filename };
	if (not in.is_open()) {
		return make_error_code(obj_cannot_open_file);
	}

	namespace fs = std::filesystem;
	const auto directory = fs::path(filename).parent_path();

	// Vertex lookup for parsing.
	// Contains one default value for each type because a face must not always specify
	// an index for normal and texture coordinated. Since the final vertex buffer needs
	// to have all three components, these default values are used in these cases.
	// (The default vertex position is not strictly needed but makes indexing easier)
	std::vector<typename vertex_components::position::type> vertices{ { 0.f, 0.f, 0.f } };
	std::vector<typename vertex_components::normal::type> normals{ { 0.f, 0.f, 0.f } };
	std::vector<typename vertex_components::tex_coord::type> tex_coords{ { 0.f, 0.f } };

	static constexpr auto num_comps = std::tuple_size_v<typename mesh<Cs...>::vertex_t>;

	// Final vertex and index buffer for OpenGL
	std::vector<typename mesh<Cs...>::vertex_t> vertex_buffer;
	std::vector<ztu::u32> index_buffer;

	// Each vertex of a face can represent a unique combination of vertex-/texture-/normal-coordinates.
	// But some combinations may occur more than once, for example on every corner of a cube 3 triangles will
	// reference the exact same corner vertex.
	// To get the best rendering performance and lowest final memory footprint these duplicates
	// need to be removed. So this sorted lookup is used to identify the aforementioned duplicates
	// and only push unique combinations to the vertex buffer.
	std::vector<indexed_vertex_id<Cs...>> vertex_ids;

	std::string use_material_name;
	mesh_loader_error::codes errc{ };
	std::string line;

	const auto push_mesh = [&]() {
		if (not vertex_buffer.empty()) {
			// Copy buffers instead of moving to keep capacity for further parsing
			// and have the final buffers be shrunk to size.
			auto& new_mesh = destination.emplace_back(vertex_buffer, index_buffer);

			if (not use_material_name.empty()) {
				const auto it = materials.find(use_material_name);
				if (it != materials.end()) {
					new_mesh.m_material = it->second;
				}
			}
		}

		vertex_buffer.clear();
		index_buffer.clear();
		vertex_ids.clear();
		use_material_name.clear();
	};

	const auto find_or_push_vertex = [&](const std::array<ztu::u32, num_comps>& comp_indices) -> ztu::isize {
		// Search through sorted lookup to check if index combination is unique
		indexed_vertex_id<Cs...> vID(comp_indices);
		const auto id_it = std::upper_bound(vertex_ids.begin(), vertex_ids.end(), vID);

		ztu::isize index;

		if (id_it != vertex_ids.begin() and *(id_it - 1) == vID) {
			index = (id_it - 1)->bufferIndex;
		} else {
			index = vID.bufferIndex = vertex_buffer.size();
			vertex_ids.insert(id_it, vID);

			auto& dst_vertex = vertex_buffer.emplace_back();

			using vertex = mesh<Cs...>::vertex;
			const auto set_vertex_comp = [&dst_vertex]<typename Component>(
				const std::vector<typename Component::type>& list,
				const ztu::u32 index
			) -> mesh_loader_error::codes {
				if (index >= list.size()) {
					return obj_face_index_out_of_range;
				}
				const auto& value = list[index];
				ztu::for_each::index<std::tuple_size_v<vertex>>(
					[&]<auto Index>() {
						if constexpr (std::is_same_v<
							Component, std::tuple_element_t<Index, vertex>>) {
							std::get<Index>(dst_vertex) = value;
							return true;
						}
						return false;
					}
				);

				return ok;
			};

			// @formatter:off unreadable if turned on
			if ((errc = set_vertex_comp.template operator()<vertex_components::position>(
					vertices, comp_indices[0]
				)) != ok or
				(errc = set_vertex_comp.template operator()<vertex_components::tex_coord>(
					tex_coords, comp_indices[1]
				)) != ok or
				(errc = set_vertex_comp.template operator()<vertex_components::normal>(
					normals, comp_indices[2]
				)) != ok
			) {
				// Discard whole face if one index is out of range
				index = -1;
			}
			// @formatter:on
		}

		return index;
	};

	while (std::getline(in, line)) {
		[[maybe_unused]] const auto found_match = parse_line(
			line,
			prefixed_parser{
				"v ", [&](const auto& param) {
					typename vertex_components::position::type position;
					auto it = param.begin();
					for (int i = 0; i < 3; i++) {
						const auto [ptr, ec] = std::from_chars(it, param.cend(), position[i]);
						if (ec != std::errc()) {
							errc = obj_malformed_vertex;
							return;
						}
						it = ptr + 1; // skip space in between components
					}
					vertices.push_back(position);
				}
			},
			prefixed_parser{
				"vt ", [&](const auto& param) {
					typename vertex_components::tex_coord::type coord;
					auto it = param.begin();
					for (int i = 0; i < 2; i++) {
						const auto [ptr, ec] = std::from_chars(it, param.cend(), coord[i]);
						if (ec != std::errc()) {
							errc = obj_malformed_texture_coordinate;
							return;
						}
						it = ptr + 1; // skip space in between components
					}
					tex_coords.push_back(coord);
				}
			},
			prefixed_parser{
				"vn ", [&](const auto& param) {
					typename vertex_components::normal::type normal;
					auto it = param.begin();
					for (int i = 0; i < 3; i++) {
						const auto [ptr, ec] = std::from_chars(it, param.cend(), normal[i]);
						if (ec != std::errc()) {
							errc = obj_malformed_normal;
							return;
						}
						it = ptr + 1; // skip space in between components
					}
					normals.push_back(normal);
				}
			},
			prefixed_parser{
				"o ", [&](const auto& param) {
					push_mesh(); // Name is currently ignored
				}
			},
			prefixed_parser{
				"f ", [&](const auto& param) {
					ztu::u32 first_index, prev_index, comp_index = 0;

					// Index for position and all the vertex components
					// Indices are set to 0 so if the obj does not hold m_data
					// for that component the fallback component at index 0 is used
					std::array<ztu::u32, num_comps> comp_indices{ };

					const char* it = param.begin();
					for (int vertex_index = 0; it <= param.end();) {
						// include an extra iteration to push the last vertex
						if (it == param.end() or *it == ' ') {

							const auto curr_index = find_or_push_vertex(comp_indices);
							if (curr_index == -1) {
								return;
							}

							if (vertex_index >= 2) {
								index_buffer.reserve(3);
								index_buffer.push_back(first_index);
								index_buffer.push_back(prev_index);
								index_buffer.push_back(curr_index);
							} else if (vertex_index == 0) {
								first_index = curr_index;
							}

							prev_index = curr_index;
							vertex_index++;
							it++;
							comp_index = 0;

						} else if (*it == '/') {
							comp_index++;
							it++;
							if (comp_index == 3) [[unlikely]] {
								errc = obj_malformed_face;
								return;
							}
						} else {
							// Implement relative indexing feature
							const auto [ptr, ec] = std::from_chars(it, param.cend(), comp_indices[comp_index]);
							if (ec != std::errc()) {
								errc = obj_malformed_face;
								// Discard whole face if one index is malformed
								return;
							}
							it = ptr;
						}
					}
				}
			},
			prefixed_parser{
				"usemtl ", [&](const auto& param) {
					use_material_name = param;
				}
			},
			prefixed_parser{
				"mtllib ", [&](const auto& param) {
					auto material_filename = fs::path(param);
					if (material_filename.is_relative()) {
						material_filename = directory / material_filename;
					}
					errc = parse_mtl(material_filename, materials, pedantic);
				}
			}
		);
		if (pedantic) {
			/*
			Even on pedantic this is too much as this parser is not feature complete.
			if (not found_match) [[unlikely]] {
			 	return make_error_code(obj_unknown_line_begin);
			}
		 	*/
			if (errc != ok) [[unlikely]] {
				return make_error_code(errc);
			}
		}
	}

	push_mesh();

	return { };
}

mesh_loader_error::codes mesh_loader::parse_mtl(
	const std::filesystem::path& filename,
	std::unordered_map<std::string, std::shared_ptr<material>>& materials,
	bool pedantic
) {

	using
	enum mesh_loader_error::codes;

	auto in = std::ifstream{ filename };
	if (not in.is_open()) {
		return mtl_cannot_open_file;
	}

	namespace fs = std::filesystem;
	const auto directory = fs::path(filename).parent_path();

	std::string curr_name;
	material curr_material;

	const auto push = [&]() {
		if (not curr_name.empty()) {
			materials.emplace(
				std::move(curr_name),
				std::make_shared<material>(std::move(curr_material))
			);
		}
	};

	mesh_loader_error::codes errc{ };
	std::string line;

	while (std::getline(in, line)) {
		errc = ok;
		parse_line(
			line,
			prefixed_parser{
				"map_Kd ", [&](const auto& param) {
					auto texture_filename = fs::path(param);
					if (texture_filename.is_relative()) {
						texture_filename = directory / texture_filename;
					}
					texture tex;
					if (texture::load(texture_filename.c_str(), tex, true)) {
						errc = mtl_cannot_open_texture;
					} else {
						curr_material.m_tex = std::make_unique<texture>(
							std::move(tex)
						);
					}
				}
			},
			prefixed_parser{
				"Kd ", [&](const auto& param) {
					auto it = param.cbegin();
					rgba_color color;

					for (int i = 0; i < 3; i++) {
						const auto [ptr, ec] = std::from_chars(it, param.cend(), color[i]);
						if (ec != std::errc()) [[unlikely]] {
							errc = mtl_malformed_color;
							return;
						}
						it = ptr + 1; // skip space in between components
					}
					color[3] = 1.0f;

					if (curr_material.m_color) {
						*curr_material.m_color = color;
					} else {
						curr_material.m_color = std::make_unique<rgba_color>(color);
					}
				}
			},
			prefixed_parser{
				"d ", [&](const auto& param) {
					float alpha;
					const auto [ptr, ec] = std::from_chars(
						param.cbegin(), param.cend(), alpha, std::chars_format::general
					);
					if (ec != std::errc()) [[unlikely]] {
						errc = mtl_malformed_color_alpha;
					} else {
						if (curr_material.m_color) {
							(*curr_material.m_color)[3] = alpha;
						} else {
							curr_material.m_color = std::make_unique<rgba_color>(0, 0, 0, alpha);
						}
					}
				}
			},
			prefixed_parser{
				"newmtl ", [&](const auto& param) {
					push();
					curr_name = param;
				}
			}
		);
		if (pedantic) {
			/*
			Even on pedantic this is too much as this parser is not feature complete.
			if (not found_match) [[unlikely]] {
			 	return obj_unknown_line_begin;
			}
		 	*/
			if (errc != ok) [[unlikely]] {
				return errc;
			}
		}
	}

	push();

	return ok;
}
