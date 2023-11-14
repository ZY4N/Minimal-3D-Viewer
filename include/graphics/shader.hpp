#pragma once

#include <filesystem>
#include <string>
#include <array>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include "util/string_literal.hpp"
#include "util/string_indexer.hpp"


template<ztu::string_literal... Parameters>
class shader {
private:
	static constexpr auto indexer = string_indexer<sizeof...(Parameters)>(Parameters...);

	[[nodiscard]] inline static std::error_code load_source(const std::filesystem::path& filename, std::string& source);

	[[nodiscard]] inline static std::error_code compile(GLenum type, const std::string& source, GLuint& shader_id);

public:
	[[nodiscard]] inline static std::error_code from_files(
		const std::filesystem::path& vertex_file,
		const std::filesystem::path& geometry_file,
		const std::filesystem::path& fragment_file,
		shader& dst
	);

	[[nodiscard]] inline static std::error_code from_sources(
		const std::string& vertex_src,
		const std::string& geometry_src,
		const std::string& fragment_src,
		shader& dst
	);

	inline shader() = default;

	inline shader(shader<Parameters...>&& other);

	inline shader(const shader<Parameters...>& other) = delete;

	inline shader<Parameters...>& operator=(shader<Parameters...>&& other) noexcept;

	inline shader<Parameters...>& operator=(const shader<Parameters...>& other) = delete;

	inline void init(GLuint program_id);

	inline void bind();

	inline void unbind();

	template<ztu::string_literal Parameter, typename T>
	inline void set(const T& value);

	~shader();

private:
	GLuint m_id{ 0 };
	std::array<GLint, sizeof...(Parameters)> valueIDs{};
};

#define INCLUDE_SHADER_IMPLEMENTATION
#include "graphics/shader.ipp"


#undef INCLUDE_SHADER_IMPLEMENTATION
