#pragma once

#include "graphics/renderable_attribute.hpp"
#include "util/rgba_color.hpp"


class color_attribute : public renderable_attribute_internal::base_renderable_attribute<"uniform_color"> {
private:
	static constexpr auto color_param{ 0 };

public:
	explicit color_attribute(const glm::fvec4& n_color) : m_color{ n_color } {
	}

	rgba_color& color() {
		return m_color;
	}

	template<ztu::string_literal... Parameters>
	inline void pre_render(shader<Parameters...>& s) const {
		s.template set<parameter<color_param>()>(m_color);
	}

	template<ztu::string_literal... Parameters>
	inline void post_render(shader<Parameters...>&) const {
	}

private:
	rgba_color m_color;
};

static_assert(renderable_attribute<color_attribute>);
