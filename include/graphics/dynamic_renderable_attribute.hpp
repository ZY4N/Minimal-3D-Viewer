#pragma once

#include <variant>
#include <memory>
#include "renderable_attributes/color_attribute.hpp"
#include "renderable_attributes/texture_attribute.hpp"
#include "util/logger.hpp"


template<renderable_attribute... Attributes>
struct dynamic_renderable_attribute {

	dynamic_renderable_attribute(const dynamic_renderable_attribute&) = default;

	dynamic_renderable_attribute(dynamic_renderable_attribute&&) = default;

	template<typename... Args>
	explicit dynamic_renderable_attribute(Args&& ... args) : attributes(std::forward<Args>(args)...) {
	}

	template<ztu::string_literal... Parameters>
	void pre_render(shader<Parameters...>& shader) {
		std::visit(
			[&](auto& attribute) {
				if (auto attr = attribute.lock()) {
					attr->pre_render(shader);
				} else {
					error<"renderable attribute holds m_data that went out of scope">();
				}
			}, attributes
		);
	}

	template<ztu::string_literal... Parameters>
	void post_render(shader<Parameters...>& shader) {
		std::visit(
			[&](auto& attribute) {
				if (auto attr = attribute.lock()) {
					attr->post_render(shader);
				} else {
					error<"renderable attribute holds m_data that went out of scope">();
				}
			}, attributes
		);
	}

	[[nodiscard]] std::size_t index() const {
		return attributes.index();
	}

	std::variant<std::weak_ptr<Attributes>...> attributes;
};



