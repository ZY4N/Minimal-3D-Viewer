#ifndef INCLUDE_SHADER_IMPLEMENTATION
#error Never include this file directly include 'shader.hpp'
#endif

#include <fstream>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util/for_each.hpp"
#include "util/logger.hpp"


template<ztu::string_literal... Parameters>
void shader<Parameters...>::init(GLuint program_id) {
	this->m_id = program_id;
	ztu::for_each::indexed_value<Parameters...>(
		[&]<auto Index, auto Parameter>() {
			valueIDs[Index] = glGetUniformLocation(m_id, Parameter.c_str());
			return false;
		}
	);
}

template<ztu::string_literal... Parameters>
shader<Parameters...>::shader(shader<Parameters...>&& other) {
	this->m_id = other.m_id;
	other.m_id = 0;
	std::copy(other.valueIDs.begin(), other.valueIDs.end(), valueIDs.begin());
}

template<ztu::string_literal... Parameters>
shader<Parameters...>& shader<Parameters...>::operator=(shader<Parameters...>&& other) noexcept {
	if (&other != this) {
		this->~shader();
		this->m_id = other.m_id;
		other.m_id = 0;
		std::copy(other.valueIDs.begin(), other.valueIDs.end(), valueIDs.begin());
	}
	return *this;
}

template<ztu::string_literal... Parameters>
std::error_code shader<Parameters...>::load_source(const std::filesystem::path& filename, std::string& source) {

	auto file = std::ifstream(filename);
	if (not file.is_open()) {
		return std::make_error_code(std::errc::no_such_file_or_directory);
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();

	if (size == 0 or size == std::numeric_limits<std::streamsize>::max()) {
		return std::make_error_code(std::errc::invalid_seek);
	}

	source.resize(size);

	file.seekg(0, std::ios::beg);
	file.read(source.data(), size);
	file.close();

	return {};
}


template<ztu::string_literal... Parameters>
std::error_code shader<Parameters...>::compile(GLenum type, const std::string& source, GLuint& shader_id) {
	shader_id = glCreateShader(type);

	// Curious choice to take lists as parameters...
	const auto first_list_element = source.c_str();
	const auto first_list_element_len = static_cast<GLint>(source.length());
	glShaderSource(shader_id, 1, &first_list_element, &first_list_element_len);
	glCompileShader(shader_id);

	GLint success;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (not success) {
		return std::make_error_code(std::errc::invalid_argument);
	}

	return {};
}


template<ztu::string_literal... Parameters>
std::error_code shader<Parameters...>::from_files(
	const std::filesystem::path& vertex_file,
	const std::filesystem::path& geometry_file,
	const std::filesystem::path& fragment_file,
	shader<Parameters...>& dst
) {
	std::string vertex_src, geometry_src, fragment_src;
	for (auto& [file, src] : {
		std::tie(vertex_file, vertex_src),
		std::tie(geometry_file, geometry_src),
		std::tie(fragment_file, fragment_src),
	}) {
		if (const auto e = load_source(file, src); e) {
			warn<"Could not load shader source file %: %">(file, e.message());
		}
	}
	return from_sources(vertex_src, geometry_src, fragment_src, dst);
}


template<ztu::string_literal... Parameters>
std::error_code shader<Parameters...>::from_sources(
	const std::string& vertexSrc,
	const std::string& geometrySrc,
	const std::string& fragmentSrc,
	shader<Parameters...>& dst
) {
	GLuint vertex_id{}, geometry_id{}, fragment_id{};
	static constexpr auto
		vertex_type{ GL_VERTEX_SHADER },
		geometry_type{ GL_GEOMETRY_SHADER },
		fragment_type{ GL_FRAGMENT_SHADER };

	const auto program_id = glCreateProgram();

	for (auto& [src, type, id, name] : {
		std::tie(vertexSrc, vertex_type, vertex_id, "vertex  "),
		std::tie(geometrySrc, geometry_type, geometry_id, "geometry"),
		std::tie(fragmentSrc, fragment_type, fragment_id, "fragment")
	}) {
		if (src.empty()) {
			warn<"Proceeding with default % shader">(name);
		} else if (const auto e = compile(type, src, id); e) {
			GLint log_length{};
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);
			auto log = std::string(log_length, ' ');
			glGetShaderInfoLog(id, log_length, nullptr, log.data());
			warn<"Failed compiling % shader:\n%\n">(name, log);
			warn<"Proceeding with default shader.">();
		} else {
			glAttachShader(program_id, id);
		}
	}

	glLinkProgram(program_id);

	GLint success;
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (not success) {
		GLint log_length{};
		glGetShaderiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
		auto log = std::string(log_length, ' ');
		glGetProgramInfoLog(program_id, log_length, nullptr, log.data());
		return std::make_error_code(std::errc::io_error);
	}

	glUseProgram(0);

	for (const auto& id : { vertex_id, geometry_id, fragment_id }) {
		if (id) {
			glDeleteShader(id);
		}
	}

	dst.init(program_id);

	return {};
}

template<ztu::string_literal... Parameters>
template<ztu::string_literal Parameter, typename T>
void shader<Parameters...>::set(const T& value) {
	constexpr auto value_index_opt = indexer.index_of(Parameter);
	GLint valueID;
	if constexpr (value_index_opt) {
		valueID = valueIDs[value_index_opt.value()];
	} else {
		// constexpr auto _using_uncached_uniform = Parameter.c_str(); // warning
		valueID = glGetUniformLocation(m_id, Parameter.c_str());
	}

	//bind();
	if constexpr (std::same_as<T, glm::mat4x4>)
		glUniformMatrix4fv(valueID, 1, false, glm::value_ptr(value));
	else if constexpr (std::same_as<T, glm::mat3x3>)
		glUniformMatrix3fv(valueID, 1, false, glm::value_ptr(value));
	else if constexpr (std::same_as<T, glm::fvec4>)
		glUniform4fv(valueID, 1, glm::value_ptr(value));
	else if constexpr (std::same_as<T, glm::fvec3>)
		glUniform3fv(valueID, 1, glm::value_ptr(value));
	else if constexpr (std::same_as<T, glm::fvec2>)
		glUniform2fv(valueID, 1, glm::value_ptr(value));
	else if constexpr (std::same_as<T, float>)
		glUniform1f(valueID, value);
	else if constexpr (std::same_as<T, int>)
		glUniform1i(valueID, value);
	else {
		T::_unknown_shader_uniform;
	}
	//unbind();
}

template<ztu::string_literal... Parameters>
shader<Parameters...>::~shader() {
	if (m_id) {
		glDeleteProgram(m_id);
	}
}

template<ztu::string_literal... Parameters>
void shader<Parameters...>::bind() {
	glUseProgram(m_id);
}

template<ztu::string_literal... Parameters>
void shader<Parameters...>::unbind() {
	glUseProgram(0);
}
