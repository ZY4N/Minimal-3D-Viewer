#pragma once

#include <memory>
#include <graphics/texture.hpp>
#include <util/rgba_color.hpp>

#include <graphics/renderable_attributes/texture_attribute.hpp>
#include <graphics/renderable_attributes/color_attribute.hpp>


struct material {
	material() = default;

	material(const material&) = delete;

	material& operator=(const material&) = delete;

	material(material&& other) noexcept :
		m_color(std::move(other.m_color)),
		m_tex(std::move(other.m_tex)) {
	}

	void init_attributes() {
		if (m_color and not m_color_attribute) {
			m_color_attribute = std::make_shared<color_attribute>(*m_color);
		}
		if (m_tex and not m_texture_attribute) {
			m_texture_attribute = std::make_shared<texture_attribute>(*m_tex);
		}
	}

	std::unique_ptr<rgba_color> m_color{};
	std::unique_ptr<texture> m_tex{};

	std::shared_ptr<color_attribute> m_color_attribute{};
	std::shared_ptr<texture_attribute> m_texture_attribute{};
};
