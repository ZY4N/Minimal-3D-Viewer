#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <charconv>
#include <functional>
#include <util/uix.hpp>
#include <util/pack.hpp>
#include <util/function.hpp>
#include <util/string_literal.hpp>
#include <util/for_each.hpp>


namespace ztu {

static constexpr char NO_SHORT_FLAG = '\0';

namespace arx_internal {

template<typename F, typename T>
concept parsing_function = (
	ztu::callable<F, std::optional<T>, std::string_view> and
		(not std::is_class_v<F> or std::is_empty_v<F>)
);

} // namespace arx_internal

template<
	char ShortName,
	ztu::string_literal LongName,
	typename T = bool,
	auto Parse = std::nullptr_t{}
> requires (
std::same_as<std::nullptr_t, decltype(Parse)> or
	arx_internal::parsing_function<decltype(Parse), T>
)
struct arx_flag {
	static constexpr auto short_name = ShortName;
	static constexpr auto long_name = LongName;
	using type = T;
	static constexpr auto parse = Parse;
};

namespace arx_parsers {

template<std::integral Type = int, int Base = 10>
requires (Base > 0)
[[nodiscard]] inline std::optional<Type> integer(const std::string_view& str);

template<std::floating_point Type = float, std::chars_format Format = std::chars_format::general>
[[nodiscard]] inline std::optional<Type> floating_point(const std::string_view& str);

} // namespace arx_parsers

namespace arx_internal {

template<ztu::string_literal LongName, class... Flags>
struct flag_by_name {
	template<class Flag>
	struct pred : std::conditional_t<
		Flag::long_name == LongName,
		std::true_type,
		std::false_type
	> {
	};
	using type = ztu::find<pred, Flags...>;
};

template<ztu::string_literal LongName, class... Flags>
using flag_by_name_t = flag_by_name<LongName, Flags...>::type;

template<ztu::string_literal LongName, class... Flags>
using flag_type_by_name_t = flag_by_name_t<LongName, Flags...>::type;

} // namespace arx_internal


template<class... Flags>
class arx {
private:
	static constexpr auto short_flag_prefix = std::string_view{ "-" };
	static constexpr auto long_flag_prefix = std::string_view{ "--" };
	static constexpr auto UNBOUND_ARGUMENT = ztu::isize_max;

public:
	inline arx(int num_args, const char* const* args);

	template<ztu::string_literal LongName>
	[[nodiscard]] inline std::optional<arx_internal::flag_type_by_name_t<LongName, Flags...>> get() const;

	[[nodiscard]] inline std::optional<std::string_view> get(ztu::isize position) const;

	[[nodiscard]] inline ztu::isize num_positional() const;

protected:
	[[nodiscard]] inline std::optional<std::string_view> find_flag_value(ztu::isize flag_index) const;

	template<ztu::string_literal LongName>
	[[nodiscard]] inline std::optional<arx_internal::flag_type_by_name_t<LongName, Flags...>>
	parse_value(const std::string_view& value_str) const;

private:
	std::vector<std::pair<ztu::isize, std::string_view>> m_arguments;
	ztu::isize m_unbound_begin;
};


namespace arx_parsers {

template<std::integral Type, int Base>
requires (Base > 0)
[[nodiscard]] inline std::optional<Type> integer(const std::string_view& str) {
	Type value{};
	const auto [ptr, ec] = std::from_chars(str.begin(), str.end(), value, Base);
	if (ec == std::errc() and ptr == str.end()) {
		return value;
	}
	return std::nullopt;
}

template<std::floating_point Type, std::chars_format Format>
[[nodiscard]] inline std::optional<Type> floating_point(const std::string_view& str) {
	Type value{};
	const auto [ptr, ec] = std::from_chars(str.begin(), str.end(), value, Format);
	if (ec == std::errc() and ptr == str.end()) {
		return value;
	}
	return std::nullopt;
}

} // namespace arx_parsers

template<class... Flags>
arx<Flags...>::arx(int num_args, const char* const* args) : m_arguments(std::max(num_args - 1, 0)) {
	for (int i = 1; i < num_args; i++) {
		const auto argument = std::string_view{ args[i] };

		const auto found_match = for_each::indexed_type<Flags...>(
			[&]<auto Index, typename Flag>() {
				if ((
					Flag::short_name != NO_SHORT_FLAG and
						argument.length() == short_flag_prefix.length() + 1 and
						argument.starts_with(short_flag_prefix) and
						argument[short_flag_prefix.length()] == Flag::short_name
				) or (
					argument.length() == long_flag_prefix.length() + Flag::long_name.length() and
						argument.starts_with(long_flag_prefix) and
						argument.substr(long_flag_prefix.length()) == Flag::long_name
				)) {
					if constexpr (std::same_as<typename Flag::type, bool>) {
						m_arguments.emplace_back(Index, argument);
					} else {
						if (i + 1 < num_args) {
							const auto value = std::string_view{ args[++i] };
							m_arguments.emplace_back(Index, value);
						}
					}
					return true;
				}
				return false;
			}
		);

		if (not found_match) {
			m_arguments.emplace_back(UNBOUND_ARGUMENT, argument);
		}
	}

	std::sort(
		m_arguments.begin(), m_arguments.end(), [](const auto& a, const auto& b) -> bool {
			return a.first < b.first;
		}
	);

	const auto first_unbound = std::find_if(
		m_arguments.begin(), m_arguments.end(), [](const auto& a) {
			return a.first == UNBOUND_ARGUMENT;
		}
	);

	const auto num_bound = m_arguments.end() - first_unbound;

	const auto last = std::unique(
		std::reverse_iterator(first_unbound), m_arguments.rend(), [](const auto& a, const auto& b) {
			return a.first == b.first;
		}
	).base();

	m_arguments.erase(m_arguments.begin(), last);

	m_unbound_begin = m_arguments.size() - num_bound;
};


template<class... Flags>
std::optional<std::string_view> arx<Flags...>::find_flag_value(isize flag_index) const {
	const auto begin = m_arguments.begin();
	const auto end = begin + m_unbound_begin;
	const auto it = std::lower_bound(
		begin, end, flag_index, [](const auto& a, const auto& b) {
			return a.first < b;
		}
	);
	if (it != end and it->first == flag_index) {
		return { it->second };
	}
	return std::nullopt;
}


template<class... Flags>
template<string_literal LongName>
std::optional<arx_internal::flag_type_by_name_t<LongName, Flags...>> arx<Flags...>::parse_value(
	const std::string_view& value_str
) const {

	using Flag = arx_internal::flag_by_name_t<LongName, Flags...>;
	using Type = Flag::type;
	using opt_t = std::optional<Type>;

	opt_t ret;

	// Use custom parser if provided and if not try using a default parser
	if constexpr (
		requires(const std::string_view str) {
			{
			std::invoke(Flag::parse, str)
			} -> std::same_as<opt_t>;
		}
		) {
		return std::invoke(Flag::parse, value_str);
	} else if constexpr (std::integral<Type> && not std::same_as<Type, bool>) {
		return arx_parsers::integer(value_str);
	} else if constexpr (std::floating_point<Type>) {
		return arx_parsers::floating_point(value_str);
	} else if constexpr (std::same_as<std::string_view, Type>) {
		return value_str;
	} else if constexpr (std::same_as<std::string, Type>) {
		return std::string(value_str);
	} else {
		Type::__cannot_parse_this_type;
	}

	return ret;
}


template<class... Flags>
template<string_literal LongName>
std::optional<arx_internal::flag_type_by_name_t<LongName, Flags...>> arx<Flags...>::get() const {
	using Flag = arx_internal::flag_by_name_t<LongName, Flags...>;
	static constexpr auto index = index_of<Flag, Flags...>;

	if (index < sizeof...(Flags)) {
		const auto value_opt = find_flag_value(index);
		if constexpr (std::same_as<typename Flag::type, bool>) {
			return value_opt.has_value();
		} else if (value_opt) {
			return parse_value<LongName>(*value_opt);
		}
	}

	return std::nullopt;
}

template<class... Flags>
inline isize arx<Flags...>::num_positional() const {
	return m_arguments.size() - m_unbound_begin;
}

template<class... Flags>
std::optional<std::string_view> arx<Flags...>::get(isize position) const {
	if (0 <= position and position < num_positional()) {
		return { m_arguments[m_unbound_begin + position].second };
	}
	return std::nullopt;
}

} // namespace ztu

