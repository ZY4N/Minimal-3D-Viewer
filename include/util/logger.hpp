#pragma once

#include <array>
#include <iostream>
#include <algorithm>
#include <string_view>

#include "util/uix.hpp"
#include "util/string_literal.hpp"


template<typename... Args>
inline void error(Args&& ... args);

template<typename... Args>
inline void warn(Args&& ... args);

template<typename... Args>
inline void log(Args&& ... args);

template<typename... Args>
inline void debug(Args&& ... args);

template<typename... Args>
inline void info(Args&& ... args);

template<ztu::string_literal Format, typename... Args>
inline void error(Args&& ... args);

template<ztu::string_literal Format, typename... Args>
inline void warn(Args&& ... args);

template<ztu::string_literal Format, typename... Args>
inline void log(Args&& ... args);

template<ztu::string_literal Format, typename... Args>
inline void debug(Args&& ... args);

template<ztu::string_literal Format, typename... Args>
inline void info(Args&& ... args);

class logger {
public:
	enum class level : ztu::u8 {
		MUTE = UINT8_MAX,
		ERROR = 0,
		WARN = 1,
		LOG = 2,
		DBG = 3,
		INFO = 4
	};

	inline static level global_level = logger::level::INFO;
	static constexpr char fmt_symbol = '%';

	inline logger();

	inline constexpr logger(level n_level);

	inline static void set_global_log_level(level n_level);

	inline void set_log_level(level n_level);


	template<level Level, typename... Args>
	inline static void println(std::ostream& out, level threshold, const std::string_view& format, Args&& ... args);

	template<level Level, ztu::string_literal Format, typename... Args>
	inline static constexpr void constexpr_println(std::ostream& out, level threshold, Args&& ... args);


	template<ztu::string_literal FMFormatT, typename... Args>
	inline void error(Args&& ... args) const;

	template<ztu::string_literal Format, typename... Args>
	inline void warn(Args&& ... args) const;

	template<ztu::string_literal Format, typename... Args>
	inline void log(Args&& ... args) const;

	template<ztu::string_literal Format, typename... Args>
	inline void debug(Args&& ... args) const;

	template<ztu::string_literal Format, typename... Args>
	inline void info(Args&& ... args) const;


	template<typename... Args>
	inline void error(Args&& ... args) const;

	template<typename... Args>
	inline void warn(Args&& ... args) const;

	template<typename... Args>
	inline void log(Args&& ... args) const;

	template<typename... Args>
	inline void debug(Args&& ... args) const;

	template<typename... Args>
	inline void info(Args&& ... args) const;

private:
	level m_level;

public:
	//----------[ implementation ]----------//

	enum class colors : ztu::u8 {
		RESET = 0,
		BLACK = 30,
		RED = 31,
		GREEN = 32,
		YELLOW = 33,
		BLUE = 34,
		MAGENTA = 35,
		CYAN = 36,
		WHITE = 37
	};

	using color_string = std::pair<colors, std::string_view>;
	static constexpr std::array<color_string, 5> colored_level_names{
		color_string{ colors::RED, "error" },
		color_string{ colors::YELLOW, "warning" },
		color_string{ colors::WHITE, "log" },
		color_string{ colors::BLUE, "debug" },
		color_string{ colors::MAGENTA, "info" }
	};

	template<ztu::string_literal Color>
	static inline void print_impl(
		std::ostream& out, const std::string_view& format
	);

	template<ztu::string_literal Color, typename Arg, typename... Rest>
	static inline void print_impl(
		std::ostream& out, const std::string_view& format,
		Arg&& arg, Rest&& ... rest
	);

	template<
		ztu::string_literal Color,
		ztu::string_literal Format,
		size_t FormatIndex
	>
	inline static constexpr void constexpr_print_impl(std::ostream& out);

	template<
		ztu::string_literal Color,
		ztu::string_literal Format,
		size_t FormatIndex,
		typename Arg,
		typename... Rest
	>
	inline static constexpr void constexpr_print_impl(std::ostream& out, Arg&& arg, Rest&& ... rest);

	template<logger::colors Color, ztu::string_literal Name>
	[[nodiscard]] static constexpr auto create_prefix();
};

namespace logger_detail {
constexpr ztu::u32 num_digits(ztu::u32 n, uint32_t base = 10) {
	if (n < base) {
		return 1;
	}
	ztu::u32 digits = 0;
	while (n > 0) {
		n /= base;
		digits++;
	}
	return digits;
}

template<ztu::u32 N, ztu::u32 B = 10>
constexpr auto int_to_str() {
	constexpr auto length = num_digits(N, B);
	auto str = ztu::string_literal<length + 1>(' ', length);
	auto index = length - 1;
	ztu::u32 num = N;
	do {
		str[index] = (char)('0' + num % B);
		num /= B;
	} while (index--);
	return str;
}

template<logger::colors color, bool bright>
constexpr auto create_ANSI_color() {
	using namespace ztu::string_literals;
	constexpr auto color_str = "\x1B["_sl + int_to_str<static_cast<ztu::u8>(color)>();
	if constexpr (bright) {
		return color_str + ";1m"_sl;
	} else {
		return color_str + "m"_sl;
	}
}

static constexpr auto reset_color = create_ANSI_color<logger::colors::RESET, false>();
}

template<logger::colors Color, ztu::string_literal Name>
constexpr auto logger::create_prefix() {
	constexpr auto longest_name = std::max_element(
		colored_level_names.begin(), colored_level_names.end(),
		[](const auto& a, const auto& b) {
			return a.second.size() < b.second.size();
		}
	)->second.size();
	constexpr auto color_str = logger_detail::create_ANSI_color<Color, true>();
	constexpr auto padding_size = longest_name + 1 - Name.length();
	constexpr auto padding = ztu::string_literal<padding_size + 1>(' ', padding_size);
	using namespace ztu::string_literals;
	return "["_sl + color_str + Name + logger_detail::reset_color + "]"_sl + padding;
}


template<ztu::string_literal Color>
void logger::print_impl(
	std::ostream& out, const std::string_view& format
) {
	out << format;
}

template<ztu::string_literal Color, typename Arg, typename... Rest>
void logger::print_impl(
	std::ostream& out, const std::string_view& format,
	Arg&& arg, Rest&& ... rest
) {
	if (format.empty()) {
		return;
	}
	const auto next_index = format.find(fmt_symbol);
	if (next_index == std::string::npos) {
		out << format;
	} else {
		out.write(format.data(), static_cast<std::streamsize>(next_index));
		out << Color << std::forward<Arg>(arg) << logger_detail::reset_color;
		print_impl<Color>(out, format.substr(next_index + sizeof(fmt_symbol)), std::forward<Rest>(rest)...);
	}
}

template<
	ztu::string_literal Color,
	ztu::string_literal Format,
	size_t FormatIndex
>
constexpr void logger::constexpr_print_impl(std::ostream& out) {
	static_assert(
		Format.find(fmt_symbol, FormatIndex) == Format.length(), "Fewer m_arguments than specified in format string."
	);
	out.write(Format.c_str() + FormatIndex, Format.length() - FormatIndex);
}

template<
	ztu::string_literal Color,
	ztu::string_literal Format,
	size_t FormatIndex,
	typename Arg,
	typename... Rest
>
constexpr void logger::constexpr_print_impl(std::ostream& out, Arg&& arg, Rest&& ... rest) {
	constexpr auto format_length = Format.length();
	constexpr auto next_index = Format.find(fmt_symbol, FormatIndex);
	out.write(Format.c_str() + FormatIndex, next_index - FormatIndex);
	static_assert(next_index < format_length, "More m_arguments than specified in format string.");
	out << Color << std::forward<Arg>(arg) << logger_detail::reset_color;
	constexpr_print_impl<Color, Format, next_index + sizeof(fmt_symbol)>(
		out, std::forward<Rest>(rest)...
	);
}


logger::logger() : m_level{ global_level } {
}

constexpr logger::logger(level n_level) : m_level{ n_level } {
}

void logger::set_global_log_level(logger::level n_level) {
	global_level = n_level;
}

void logger::set_log_level(logger::level n_level) {
	m_level = n_level;
}

template<logger::level Level, typename... Args>
void logger::println(std::ostream& out, level threshold, const std::string_view& format, Args&& ... args) {
	constexpr auto colored_level = colored_level_names[static_cast<size_t>(Level)];
	constexpr auto color_str = logger_detail::create_ANSI_color<colored_level.first, false>();
	constexpr auto name_literal = ztu::string_literal<colored_level.second.length() + 1>(colored_level.second);
	constexpr auto prefix = create_prefix<colored_level.first, name_literal>();
	if (static_cast<ztu::u8>(threshold) >= static_cast<ztu::u8>(Level)) {
		out << prefix;
		print_impl<color_str>(out, format, std::forward<Args>(args)...);
		out << std::endl;
	}
}


template<logger::level Level, ztu::string_literal Format, typename... Args>
constexpr void logger::constexpr_println(std::ostream& out, logger::level threshold, Args&& ... args) {
	constexpr auto colored_level = colored_level_names[static_cast<size_t>(Level)];
	constexpr auto color_str = logger_detail::create_ANSI_color<colored_level.first, false>();
	constexpr auto name_literal = ztu::string_literal<colored_level.second.length() + 1>(colored_level.second);
	constexpr auto prefix = create_prefix<colored_level.first, name_literal>();
	if (static_cast<ztu::u8>(threshold) >= static_cast<ztu::u8>(Level)) {
		out << prefix;
		constexpr_print_impl<color_str, Format, 0>(out, std::forward<Args>(args)...);
		out << std::endl;
	}
}


template<typename... Args>
void logger::error(Args&& ... args) const {
	println_impl<level::ERROR>(std::cerr, m_level, std::forward<Args>(args)...);
}

template<typename... Args>
void logger::warn(Args&& ... args) const {
	println_impl<level::WARN>(std::cerr, m_level, std::forward<Args>(args)...);
}

template<typename... Args>
void logger::log(Args&& ... args) const {
	println_impl<level::LOG>(std::cout, m_level, std::forward<Args>(args)...);
}

template<typename... Args>
void logger::debug(Args&& ... args) const {
	println_impl<level::DBG>(std::cout, m_level, std::forward<Args>(args)...);
}

template<typename... Args>
void logger::info(Args&& ... args) const {
	println_impl<level::INFO>(std::cout, m_level, std::forward<Args>(args)...);
}


template<ztu::string_literal Format, typename... Args>
void logger::error(Args&& ... args) const {
	constexpr_println<level::ERROR, Format>(std::cerr, m_level, std::forward<Args>(args)...);
}

template<ztu::string_literal Format, typename... Args>
void logger::warn(Args&& ... args) const {
	constexpr_println<level::WARN, Format>(std::cerr, m_level, std::forward<Args>(args)...);
}

template<ztu::string_literal Format, typename... Args>
void logger::log(Args&& ... args) const {
	constexpr_println<level::LOG, Format>(std::cout, m_level, std::forward<Args>(args)...);
}

template<ztu::string_literal Format, typename... Args>
void logger::debug(Args&& ... args) const {
	constexpr_println<level::DBG, Format>(std::cout, m_level, std::forward<Args>(args)...);
}

template<ztu::string_literal Format, typename... Args>
void logger::info(Args&& ... args) const {
	constexpr_println<level::INFO, Format>(std::cout, m_level, std::forward<Args>(args)...);
}


template<typename... Args>
void error(Args&& ... args) {
	logger::println<logger::level::ERROR>(
		std::cerr, logger::global_level, std::forward<Args>(args)...
	);
}

template<typename... Args>
void warn(Args&& ... args) {
	logger::println<logger::level::WARN>(
		std::cerr, logger::global_level, std::forward<Args>(args)...
	);
}

template<typename... Args>
void log(Args&& ... args) {
	logger::println<logger::level::LOG>(
		std::cout, logger::global_level, std::forward<Args>(args)...
	);
}

template<typename... Args>
void debug(Args&& ... args) {
	logger::println<logger::level::DBG>(
		std::cout, logger::global_level, std::forward<Args>(args)...
	);
}

template<typename... Args>
void info(Args&& ... args) {
	logger::println<logger::level::INFO>(
		std::cout, logger::global_level, std::forward<Args>(args)...
	);
}

template<ztu::string_literal Format, typename... Args>
void error(Args&& ... args) {
	logger::constexpr_println<logger::level::ERROR, Format>(
		std::cerr, logger::global_level, std::forward<Args>(args)...
	);
}

template<ztu::string_literal Format, typename... Args>
void warn(Args&& ... args) {
	logger::constexpr_println<logger::level::WARN, Format>(
		std::cerr, logger::global_level, std::forward<Args>(args)...
	);
}

template<ztu::string_literal Format, typename... Args>
void log(Args&& ... args) {
	logger::constexpr_println<logger::level::LOG, Format>(
		std::cout, logger::global_level, std::forward<Args>(args)...
	);
}

template<ztu::string_literal Format, typename... Args>
void debug(Args&& ... args) {
	logger::constexpr_println<logger::level::DBG, Format>(
		std::cout, logger::global_level, std::forward<Args>(args)...
	);
}

template<ztu::string_literal Format, typename... Args>
void info(Args&& ... args) {
	logger::constexpr_println<logger::level::INFO, Format>(
		std::cout, logger::global_level, std::forward<Args>(args)...
	);
}
