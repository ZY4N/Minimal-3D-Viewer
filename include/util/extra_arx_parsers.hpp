#pragma once

#include <string_view>
#include <optional>
#include <charconv>
#include <glm/glm.hpp>

namespace extra_arx_parsers {

	template<int Count, typename T, glm::qualifier Q>
		requires (Count > 0)
	[[nodiscard]] inline std::optional<glm::vec<Count, T>> glm_vec(const std::string_view& str) {
		glm::vec<Count, T, Q> vec{};
		auto it = str.cbegin();
		for (int i = 0; i < Count; i++) {
			const auto [ptr, ec] = std::from_chars(it, str.cend(), vec[i], std::chars_format::general);
			if (ec != std::errc()) {
				return std::nullopt;
			}
			it = ptr + 1; // skip space in between components
		}
		if (it < str.cend()) {
			return std::nullopt;
		}
		return vec;
	}

}
