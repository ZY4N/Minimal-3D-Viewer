#pragma once

#include "graphics/renderable_attribute.hpp"


class point_size_attribute : public renderable_attribute_internal::base_renderable_attribute<"point_size"> {
private:
	static constexpr auto size_param{ 0 };

public:
	explicit point_size_attribute(const float& n_size) : m_size{ n_size } {
	}

	float& point_size() {
		return m_size;
	}

	template<ztu::string_literal... Parameters>
	inline void pre_render(shader<Parameters...>& s) const {
		s.template set<parameter<size_param>()>(m_size);
	}

	template<ztu::string_literal... Parameters>
	inline void post_render(shader<Parameters...>&) const {
	}

private:
	float m_size;
};

static_assert(renderable_attribute<color_attribute>);
