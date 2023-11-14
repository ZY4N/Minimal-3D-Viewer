#pragma once

#include <array>
#include <optional>
#include <span>
#include <string_view>
#include <algorithm>
#include <util/uix.hpp>
#include <util/for_each.hpp>

template<ztu::usize NumKeys>
class string_indexer {
private:
	struct index_type {
		unsigned int hash;
		ztu::usize index;

		[[nodiscard]] inline constexpr auto operator<=>(const index_type &other) const {
			return hash <=> other.hash;
		}

		[[nodiscard]] inline constexpr auto operator==(const index_type &other) const {
			return hash == other.hash;
		}

		[[nodiscard]] inline constexpr auto operator<=>(const unsigned other_hash) const {
			return hash <=> other_hash;
		}
	};

	[[nodiscard]] inline constexpr static unsigned hash(std::span<const char> str);

public:
	template<typename... Ts>
		requires (sizeof...(Ts) == NumKeys)
	consteval explicit string_indexer(const Ts&... keys) noexcept;

	[[nodiscard]] inline constexpr std::optional<ztu::usize> index_of(std::span<const char> str) const;

	[[nodiscard]] inline constexpr std::optional<std::string_view> name_of(ztu::usize index) const;

	[[nodiscard]] inline constexpr std::span<const std::string_view> keys() const;

private:
	std::array<index_type, NumKeys> m_lookup{};
	std::array<std::string_view, NumKeys> m_keys{};
};

template<ztu::usize NumKeys>
[[nodiscard]] inline constexpr unsigned string_indexer<NumKeys>::hash(std::span<const char> str) {

	unsigned prime = 0x1000193;
	unsigned hashed = 0x811c9dc5;

	for (const auto &c : str) {
		hashed = hashed ^ c;
		hashed *= prime;
	}

	return hashed;
}

template<ztu::usize NumKeys>
template<typename... Ts> requires (sizeof...(Ts) == NumKeys)
consteval string_indexer<NumKeys>::string_indexer(const Ts&... keys) noexcept {

	ztu::for_each::indexed_argument([&]<auto Index>(const auto &key) {
		// Since std::string_view does only truncate the '\0' of strings in the 'const char*' constructor
		// and does not deem otherwise equal views of truncated and untruncated strings equal,
		// all strings need to be truncated before constructing the view.
		const auto begin = std::begin(key), end = std::end(key);
		m_keys[Index] = { begin, std::find(begin, end, '\0') };
		m_lookup[Index] = { hash(m_keys[Index] ), Index };
		return false;
	}, keys...);

	std::sort(m_lookup.begin(), m_lookup.end());

	auto it = m_lookup.begin();
	while ((it = std::adjacent_find(it, m_lookup.end())) != m_lookup.end()) {
		const auto match = it->hash;
		for (auto it_a = it + 1; it_a != m_lookup.end() && it_a->hash == match; it_a++) {
			const auto &key_a = m_keys[it_a->index];
			for (auto it_b = it; it_b != it_a; it_b++) {
				const auto &key_b = m_keys[it_b->index];
				if (key_a == key_b) {
					throw std::logic_error("Duplicate keys");
				}
			}
		}
	}
}

template<ztu::usize NumKeys>
[[nodiscard]] inline constexpr std::optional<ztu::usize> string_indexer<NumKeys>::index_of(std::span<const char> str) const {
	const auto sv = std::string_view(str.begin(), std::find(str.begin(), str.end(), '\0'));

	const auto hashed = hash(sv);
	const auto it = std::lower_bound(m_lookup.begin(), m_lookup.end(), hashed);

	if (it == m_lookup.end() or hashed != it->hash)
		return std::nullopt;

	do [[unlikely]] {
		const auto candidate_index = it->index;
		if (m_keys[candidate_index] == sv) [[likely]] {
			return candidate_index;
		}
	} while (it < m_lookup.end() && it->hash == hashed);

	return std::nullopt;
}

template<ztu::usize NumKeys>
[[nodiscard]] inline constexpr std::optional<std::string_view> string_indexer<NumKeys>::name_of(ztu::usize index) const {
	if (index < NumKeys) {
		return m_keys[index];
	} else {
		return std::nullopt;
	}
}

template<ztu::usize NumKeys>
[[nodiscard]] inline constexpr std::span<const std::string_view> string_indexer<NumKeys>::keys() const {
	return { m_keys };
}
