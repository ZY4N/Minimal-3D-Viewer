#pragma once

#include <SFML/OpenGL.hpp>
#include "graphics/renderable_attribute.hpp"
#include "graphics/texture.hpp"


struct texture_attribute : public renderable_attribute_internal::base_renderable_attribute<"color_merge"> {
	GLuint m_texture_id{ 0 };

	texture_attribute(const texture& tex) {
		glGenTextures(1, &m_texture_id);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<int>(tex.width()), static_cast<int>(tex.height()), 0, GL_RGBA,
			GL_UNSIGNED_BYTE, tex.data()
		);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	texture_attribute(const texture_attribute&) = delete;

	texture_attribute& operator=(const texture_attribute&) = delete;

	~texture_attribute() {
		glDeleteTextures(1, &m_texture_id);
	}

	template<ztu::string_literal... Parameters>
	inline void pre_render(shader<Parameters...>& s) const {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}

	template<ztu::string_literal... Parameters>
	inline void post_render(shader<Parameters...>& s) const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

static_assert(renderable_attribute<texture_attribute>);
