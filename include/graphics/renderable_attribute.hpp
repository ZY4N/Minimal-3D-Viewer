#pragma once

#include <tuple>
#include <concepts>
#include "util/uix.hpp"
#include "util/string_literal.hpp"
#include "graphics/shader.hpp"


template<class T>
concept renderable_attribute = requires(T& effect, typename T::minimal_shader& shader) {
	{
	effect.pre_render(shader)
	} -> std::same_as<void>;
	{
	effect.post_render(shader)
	} -> std::same_as<void>;
};

namespace renderable_attribute_internal {

template<ztu::string_literal... Parameters>
struct base_renderable_attribute {
private:
	static constexpr auto parameters = std::tuple{ Parameters... };

protected:
	template<ztu::usize Index>
	static consteval auto& parameter() {
		return std::get<Index>(parameters);
	}

public:
	using minimal_shader = shader<Parameters...>;
};

} // namespace mesh_effect_internal