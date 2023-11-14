#ifndef INCLUDE_POINT_CLOUD_LOADER_IMPLEMENTATION
#error Never include this file directly include 'point_cloud_loader.hpp'
#endif


#include <fstream>
#include <charconv>
#include <glm/gtx/euler_angles.hpp>
#include "util/logger.hpp"


#ifdef __linux__
#include <sys/mman.h>
#include <fcntl.h>


#define USE_MMAP_FOR_FILE_LOAD
#endif


namespace point_cloud_loader {

std::error_code analyze_3dtk_file(
	const std::filesystem::path& filename,
	ztu::u32& num_floats,
	std::chars_format& format
) {
	auto in = std::ifstream(filename);

	if (not in.is_open()) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}

	std::string line;
	std::getline(in, line);

	auto begin = &*line.cbegin();
	auto end = &*line.cend();

	format = std::chars_format::general;
	num_floats = 0;
	float ignore_num;

	for (auto it = begin; it < end; it++) { // skip space in between components
		it += *it == '-' or *it == '+';

		std::chars_format current_format;
		if (*it == '0' and it + 1 < end and *(it + 1) == 'x') {
			it += 2; // skip 0x
			current_format = std::chars_format::hex;
		} else {
			current_format = std::chars_format::general;
		}

		if (it == begin and current_format != format) {
			return std::make_error_code(std::errc::invalid_argument);
		}

		const auto [next_it, err] = std::from_chars(it, end, ignore_num, current_format);
		if (err != std::errc()) {
			return std::make_error_code(err);
		}

		it = next_it;
		format = current_format;
		num_floats++;
	}

	return {};
}

template<bool Reflectance, bool Hex>
std::error_code load_from_3dtk_file(
	const std::filesystem::path& base_filename,
	std::vector<basic_vertex>& basic_points,
	std::vector<reflectance_vertex>& reflectance_points
) {

	auto pose_filename = base_filename, point_filename = base_filename;
	pose_filename.replace_extension(".pose");
	point_filename.replace_extension(".3d");

	namespace fs = std::filesystem;

	if (not fs::exists(pose_filename) or not fs::exists(point_filename)) {
		return make_error_code(std::errc::no_such_file_or_directory);
	}

	auto transform = glm::identity<glm::mat4>();
	{
		auto in = std::ifstream(pose_filename);
		if (not in.is_open()) {
			return std::make_error_code(static_cast<std::errc>(errno));
		}

		glm::vec3 offset, rotation;
		if (
			in >> std::skipws
				>> offset[0] >> offset[1] >> offset[2]
				>> rotation[0] >> rotation[1] >> rotation[2]
			) {
			static constexpr auto to_radians = float(M_PI / 180.0);
			transform = glm::translate(transform, offset);
			transform *= glm::eulerAngleXYZ(
				rotation[0] * to_radians,
				rotation[1] * to_radians,
				rotation[2] * to_radians
			);
		} else {
			return make_error_code(std::errc::invalid_argument);
		}
	}

	{
		auto in = std::ifstream(point_filename);
		if (not in.is_open()) {
			return std::make_error_code(static_cast<std::errc>(errno));
		}

		std::string line;
		while (std::getline(in, line)) {
			glm::vec4 vec;
			auto it = &*line.cbegin();
			auto end = &*line.cend();

			static constexpr auto num_floats = Reflectance ? 4 : 3;

			for (int i = 0; i < num_floats; i++) {
				std::from_chars_result result;
				if constexpr (Hex) {
					const auto [minus, plus] = std::pair{ *it == '-', *it == '+' };
					it += plus or minus ? 3 : 2; // skip [-+]?0x
					result = std::from_chars(it, end, vec[i], std::chars_format::hex);
					if (minus) {
						vec[i] *= -1.0;
					}
				} else {
					result = std::from_chars(it, end, vec[i], std::chars_format::general);
				}
				if (result.ec != std::errc()) {
					return std::make_error_code(result.ec);
				}
				it = result.ptr + 1; // skip space in between components
			}

			if constexpr (Reflectance) {
				const auto reflectance = (vec[3] + 20.0f) / 40.0f;
				vec[3] = 1.0f;
				const auto position = transform * vec;
				reflectance_points.emplace_back(
					glm::vec3(-position[0], position[1], position[2]),
					glm::vec1(reflectance)
				);
			} else {
				vec[3] = 1.0f;
				const auto position = transform * vec;
				basic_points.emplace_back(
					glm::vec3(position[0], position[1], position[2])
				);
			}
		}
	}

	return {};
}

std::error_code load_from_3dtk_directory(
	const std::filesystem::path& path,
	std::vector<basic_point_cloud>& basic_point_cloud,
	std::vector<reflectance_point_cloud>& reflectance_point_cloud
) {
	namespace fs = std::filesystem;

	if (not fs::exists(path)) {
		return make_error_code(std::errc::no_such_file_or_directory);
	}

	for (const auto& filename : std::filesystem::directory_iterator{ path }) {
		auto file_path = reinterpret_cast<const fs::path&>(filename);
		if (file_path.extension() != ".3d") {
			continue;
		}

		std::error_code error;
		ztu::u32 num_floats{};
		std::chars_format float_format{};
		if ((error = analyze_3dtk_file(file_path.c_str(), num_floats, float_format))) {
			warn<"Skipping file %: cannot recognize format '%'">(file_path.c_str(), error.message());
			continue;
		}

		file_path.replace_extension();

		std::vector<basic_vertex> basic_points;
		std::vector<reflectance_vertex> reflectance_points;

		if (num_floats == 3 && float_format == std::chars_format::general) {
			error = load_from_3dtk_file<false, false>(file_path.c_str(), basic_points, reflectance_points);
		} else if (num_floats == 3 && float_format == std::chars_format::hex) {
			error = load_from_3dtk_file<false, true>(file_path.c_str(), basic_points, reflectance_points);
		} else if (num_floats == 4 && float_format == std::chars_format::general) {
			error = load_from_3dtk_file<true, false>(file_path.c_str(), basic_points, reflectance_points);
		} else if (num_floats == 4 && float_format == std::chars_format::hex) {
			error = load_from_3dtk_file<true, true>(file_path.c_str(), basic_points, reflectance_points);
		} else {
			warn<"Skipping file %: unknown format (num_floats: % float_format: %)">(
				file_path.c_str(),
				num_floats,
				float_format == std::chars_format::general
					? "general"
					: "hex"
			);
			continue;
		};

		if (error) {
			warn<"Skipping file %: error while parsing '%'">(
				file_path.c_str(),
				error.message()
			);
		}

		if (basic_points.empty() and reflectance_points.empty()) {
			warn<"Skipping file %: contains no m_vertices">(file_path.c_str());
		} else if (not basic_points.empty()) {
			basic_point_cloud.emplace_back(std::move(basic_points));
		} else if (not reflectance_points.empty()) {
			reflectance_point_cloud.emplace_back(std::move(reflectance_points));
		}
	}

	return {};
}


[[nodiscard]] inline std::error_code write_v1_c3d_file(
	const std::filesystem::path& filename,
	/*const glm::vec3& position,
	const glm::vec3& direction,*/
	const std::vector<reflectance_vertex>& points
) {
	if (points.size() > ztu::u32_max) {
		return std::make_error_code(std::errc::value_too_large);
	}

	auto out = std::ofstream(filename);
	if (not out.is_open()) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}

	const auto write_float = [&out](const float& f) {
		out.write(reinterpret_cast<const char*>(&f), sizeof(float));
	};

	const auto write_float_vec = [&out](const glm::vec3& vec) {
		static constexpr auto packed_size = 3 * sizeof(float);
		static_assert(sizeof(vec) == packed_size);
		out.write(reinterpret_cast<const char*>(&vec[0]), packed_size);
	};

	const auto write_be_int = [&out]<std::integral Int>(Int i) {
		const auto bytes = reinterpret_cast<char*>(&i);
		std::reverse(bytes, bytes + sizeof(Int));
		out.write(bytes, sizeof(Int));
	};

	// magic bytes
	out.put('3');
	out.put('d');

	// version
	out.put(1);

	/*// position
	write_float_vec(position);

	// direction
	write_float_vec(direction);*/

	// num components
	out.put(4);

	// num m_vertices
	write_be_int(static_cast<ztu::u32>(points.size()));

	for (const auto& point : points) {
		write_float_vec(std::get<0>(point)); // position
		write_float(std::get<1>(point)[0]); // reflectance
	}

	return {};
}

std::error_code load_v1_c3d_file(
	const std::filesystem::path& filename,
	std::vector<reflectance_vertex>& points
) {
	auto in = std::ifstream(filename);
	if (not in.is_open()) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}

	static constexpr auto header_size = std::streamoff(
		sizeof(ztu::i8) * 2 + sizeof(ztu::u8) * 2 + sizeof(ztu::u32)
	);

	in.seekg(0, std::ios::end);
	const auto size = in.tellg();

	if (size == 0 or size == std::numeric_limits<std::streamsize>::max()) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}

	if (size < header_size) {
		return std::make_error_code(std::errc::invalid_argument);
	}

	in.seekg(0, std::ios::beg);

	static constexpr auto magic_bytes = std::array{ '3', 'd', static_cast<char>(1) };

	auto actual_magic_bytes = std::array<char, 3>{};
	in.read(actual_magic_bytes.data(), actual_magic_bytes.size());

	if (actual_magic_bytes != magic_bytes) {
		return std::make_error_code(std::errc::invalid_argument);
	}

	const auto read_be_int = [&in]<std::integral Int>() {
		Int value;
		const auto bytes = reinterpret_cast<char*>(&value);
		in.read(bytes, sizeof(Int));
		std::reverse(bytes, bytes + sizeof(Int));
		return value;
	};

	const auto num_comps = static_cast<ztu::u8>(in.get());
	const auto num_vertices = read_be_int.template operator()<ztu::u32>();

	const auto full_size = header_size + (
		static_cast<std::streamoff>(num_comps) *
			static_cast<std::streamoff>(num_vertices) *
			static_cast<std::streamoff>(sizeof(float))
	);

	if (size != full_size) {
		return std::make_error_code(std::errc::invalid_argument);
	}

	points.resize(num_vertices);

#ifdef USE_MMAP_FOR_FILE_LOAD
	in.close();
	const auto fd = open(filename.c_str(), O_RDONLY);
	if (fd == -1) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}

	const auto bytes = mmap(
		nullptr,
		full_size,
		PROT_READ,
		MAP_SHARED,
		fd,
		0
	);
	close(fd);

	if (bytes == std::numeric_limits<decltype(bytes)>::max()) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}

	const auto data = reinterpret_cast<const float*>(static_cast<const ztu::u8*>(bytes) + header_size);

	for (ztu::usize i = 0; i < num_vertices; i++) {
		auto& point = points[i];
		const auto vertex = &data[i * ztu::usize(num_comps)];
		std::copy_n(
			vertex, 3,
			reinterpret_cast<float*>(&std::get<0>(point))
		);
		std::get<1>(point) = glm::vec1{ vertex[3] };
	}

	if (munmap(bytes, full_size) != 0) {
		return std::make_error_code(static_cast<std::errc>(errno));
	}
#else
	const auto read_float = [&in]() {
		float value;
		in.read(reinterpret_cast<char*>(&value), sizeof(float));
		return value;
	};

	const auto read_float_vec3 = [&in]() {
		glm::vec3 vec;
		static constexpr auto packed_size = 3 * sizeof(float);
		static_assert(sizeof(vec) == packed_size);
		in.read(reinterpret_cast<char*>(&vec[0]), packed_size);
		return vec;
	};

	for (auto& point : m_points) {
		std::get<0>(point) = read_float_vec3();
		std::get<1>(point) = glm::vec1{ read_float() };
	}
#endif

	return {};
}

} // namespace point_cloud_loader