#pragma once

#include <array>
#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cstring>
#include <cassert>

namespace ztu {

#define ztu_ic inline constexpr
#define ztu_nic [[nodiscard]] inline constexpr

#ifdef __cpp_exceptions
#define ZTU_ASSERT(exp, error) \
	if (not (exp)) [[unlikely]] throw (error);
#else
#include <stdlib.h>
#include <stdio.h>
#define ZTU_TO_STRING_IMPL(x) #x
#define ZTU_TO_STRING(x) ZTU_TO_STRING_IMPL(x)
#define ZTU_ASSERT(exp, error) \
	if (not (exp)) [[unlikely]] { \
		puts(__FILE__ ":" ZTU_TO_STRING(__LINE__) ": Assertion '" ZTU_TO_STRING(exp) "' failed."); \
		abort(); \
	}
#endif


using sl_ssize_t = std::make_signed_t<std::size_t>;

template<sl_ssize_t N>
	requires (N > 0)
struct string_literal {
	//--------------[ typedefs ]--------------//

	static constexpr auto max_size = N - 1;
	using this_type = string_literal<N>;
	using array_type = std::array<char,N>;
	using value_type = array_type::value_type;
	using size_type = sl_ssize_t;
	using difference_type = array_type::difference_type;
	using reference = array_type::reference;
	using const_reference = array_type::const_reference;
	using pointer = array_type::pointer;
	using const_pointer = array_type::const_pointer;
	using iterator = array_type::iterator;
	using const_iterator = array_type::const_iterator;
	using reverse_iterator = array_type::reverse_iterator;
	using const_reverse_iterator = array_type::const_reverse_iterator;


	//--------------[ constructors ]--------------//

	ztu_ic string_literal();

	ztu_ic string_literal(const char (&str)[N]);

	template<sl_ssize_t M>
		requires (M < N)
	ztu_ic string_literal(const char (&str)[M]);

	template<typename... Chars>
		requires (
			(1 < sizeof...(Chars) and sizeof...(Chars) < N) and
			(std::same_as<Chars, char> and ...)
		)
	ztu_ic string_literal(Chars... chars);

	ztu_ic string_literal(char c, ssize_t count = 1);

	ztu_ic string_literal(const char* str, std::size_t len);

	ztu_ic string_literal(const std::string &str);

	ztu_ic string_literal(const std::string_view &str);


	//--------------[ array interface ]--------------//

	ztu_nic reference at(size_type index);
	ztu_nic const_reference at(size_type index) const;

	ztu_nic reference operator[](size_type index);
	ztu_nic const_reference operator[](size_type index) const;

	ztu_nic reference front();
	ztu_nic const_reference front() const;

	ztu_nic reference back();
	ztu_nic const_reference back() const;

	ztu_nic pointer data() noexcept;
	ztu_nic const_pointer data() const noexcept;

	ztu_nic iterator begin() noexcept;
	ztu_nic const_iterator begin() const noexcept;
	ztu_nic const_iterator cbegin() const noexcept;

	ztu_nic iterator end() noexcept;
	ztu_nic const_iterator end() const noexcept;
	ztu_nic const_iterator cend() const noexcept;

	ztu_nic reverse_iterator rbegin() noexcept;
	ztu_nic const_reverse_iterator rbegin() const noexcept;
	ztu_nic const_reverse_iterator rcbegin() const noexcept;

	ztu_nic reverse_iterator rend() noexcept;
	ztu_nic const_reverse_iterator rend() const noexcept;
	ztu_nic const_reverse_iterator rcend() const noexcept;

	ztu_nic bool empty() const noexcept;
	ztu_nic size_type size() const noexcept;
	ztu_nic size_type unused_size() const noexcept;


	//--------------[ string interface ]--------------//

	static constexpr auto max_length = max_size;

	ztu_nic pointer c_str() noexcept;
	ztu_nic const_pointer c_str() const noexcept;

	ztu_nic std::string_view view() const;

	ztu_nic size_type length() const noexcept;

	ztu_nic size_type find(char c, ssize_t pos = 0) const;
	ztu_nic size_type find(const char *str, size_type len, ssize_t pos) const;
	ztu_nic size_type find(const char *str, ssize_t pos = 0) const;
	ztu_nic size_type find(const std::string_view &str, ssize_t pos = 0) const;
	ztu_nic size_type find(const std::string &str, ssize_t pos = 0) const;
	template<sl_ssize_t M>
	ztu_ic size_type find(const string_literal<M> &str) const;

	ztu_ic this_type& resize(size_type new_size, char fill = ' ');

	ztu_ic this_type& erase(const_iterator begin_it, const_iterator end_it);
	ztu_ic this_type& erase(size_type index, size_type count = 1);

	ztu_ic this_type& insert(size_type index, char c, size_type repeat = 1);
	ztu_ic this_type& insert(size_type index, const char *str);
	ztu_ic this_type& insert(size_type index, const char *str, size_type len);
	ztu_ic this_type& insert(size_type index, const std::string_view &str);
	ztu_ic this_type& insert(size_type index, const std::string &str);
	template<sl_ssize_t M>
	ztu_ic this_type& insert(size_type index, const string_literal<M> &str);

	ztu_ic this_type& replace(size_type index, size_type count, char c, size_type repeat = 1);
	ztu_ic this_type& replace(size_type index, size_type count, const char *str);
	ztu_ic this_type& replace(size_type index, size_type count, const char *str, size_type len);
	ztu_ic this_type& replace(size_type index, size_type count, const std::string_view &str);
	ztu_ic this_type& replace(size_type index, size_type count, const std::string &str);
	template<sl_ssize_t M>
	ztu_ic this_type& replace(size_type index, size_type count, const string_literal<M> &str);

	//--------------[ operators ]--------------//

	ztu_nic bool operator==(const std::string &str) const;
	ztu_nic bool operator==(const std::string_view &str) const;
	ztu_nic bool operator==(const char* str) const;
	template<sl_ssize_t M>
	ztu_nic bool operator==(const string_literal<M> &str) const;


	template<bool KnownToFit = false>
	ztu_ic void assign(const char* str, size_type len);

	ztu_ic this_type& operator=(const std::string &str);
	ztu_ic this_type& operator=(const std::string_view &str);
	ztu_ic this_type& operator=(const char* str);
	template<sl_ssize_t M>
	ztu_ic this_type& operator=(const string_literal<M> &str);


	ztu_ic void append(const char* str, size_type len);

	ztu_ic this_type& operator+=(const std::string &str);
	ztu_ic this_type& operator+=(const std::string_view &str);
	ztu_ic this_type& operator+=(const char* str);
	template<sl_ssize_t M>
	ztu_ic this_type& operator+=(const string_literal<M> &str);


	template<sl_ssize_t M>
	ztu_nic string_literal<M + N - 1> operator+(const string_literal<M> &str) const;

	template<sl_ssize_t M>
	inline friend std::ostream& operator<<(std::ostream &out, const string_literal<M>& str);

	array_type m_value{};
};

//--------------[ predefines ]--------------//

namespace detail {
	template<sl_ssize_t MaxLength = std::numeric_limits<sl_ssize_t>::max()>
	ztu_nic sl_ssize_t strlen(const char* str) {
		sl_ssize_t len = 0;
		while (str[len] != '\0') {
			len++;
			ZTU_ASSERT(len < MaxLength, std::invalid_argument("given string is not null terminated"));
		}
		return len;
	}
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::size() const noexcept {
	return detail::strlen(m_value.data());
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::length() const noexcept {
	return this->size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::unused_size() const noexcept {
	return max_size - this->size();
}


//--------------[ constructors ]--------------//

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::string_literal() = default;

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::string_literal(const char (&str)[N]) {
	std::copy_n(std::begin(str), N, m_value.begin());
	m_value[N - 1] = '\0';
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M> requires (M < N)
ztu_ic string_literal<N>::string_literal(const char (&str)[M]) {
	std::copy_n(std::begin(str), M, m_value.begin());
	m_value[M - 1] = '\0';
}

template<sl_ssize_t N> requires (N > 0)
template<typename... Chars>
	requires (
		(1 < sizeof...(Chars) and sizeof...(Chars) < N) and
		(std::same_as<Chars, char> and ...)
	)
ztu_ic string_literal<N>::string_literal(Chars... chars)
	: m_value{
		chars... // default initialization of array elements to 0 terminates string
	} {}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::string_literal(const char c, ssize_t count)
	: m_value{}
{
	ZTU_ASSERT(count <= max_size, std::length_error("Given count exceeds capacity."));
	std::fill_n(m_value.begin(), count, c);
}


template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::string_literal(const char* data, std::size_t size) {
	ZTU_ASSERT(size <= max_size, std::length_error("given string exceeds capacity"));
	std::copy_n(data, size, m_value.begin());
	m_value[size] = '\0';
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::string_literal(const std::string &str)
	: string_literal(str.data(), str.size()) {}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::string_literal(const std::string_view &str)
	: string_literal(str.data(), str.size()) {}

//--------------[ array interface ]--------------//

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::reference string_literal<N>::at(size_type index) {
	const auto m_length = this->length();
	ZTU_ASSERT(0 <= index and index < m_length, std::out_of_range("given index exceeds length"));
	return m_value[index];
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reference string_literal<N>::at(size_type index) const {
	const auto m_length = this->length();
	ZTU_ASSERT(0 <= index and index < m_length, std::out_of_range("given index exceeds length"));
	return m_value[index];
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::reference string_literal<N>::operator[](size_type index) {
	return m_value[index];
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reference string_literal<N>::operator[](size_type index) const {
	return m_value[index];
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::reference string_literal<N>::front() {
	return m_value.front();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reference string_literal<N>::front() const {
	return m_value.front();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::reference string_literal<N>::back() {
	return m_value[this->size() - 1];
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reference string_literal<N>::back() const {
	return m_value[this->size() - 1];
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::pointer string_literal<N>::data() noexcept {
	return m_value.data();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_pointer string_literal<N>::data() const noexcept {
	return m_value.data();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::iterator string_literal<N>::begin() noexcept {
	return m_value.begin();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_iterator string_literal<N>::begin() const noexcept {
	return m_value.begin();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_iterator string_literal<N>::cbegin() const noexcept {
	return m_value.cbegin();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::iterator string_literal<N>::end() noexcept {
	return this->begin() + this->size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_iterator string_literal<N>::end() const noexcept {
	return this->begin() + this->size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_iterator string_literal<N>::cend() const noexcept {
	return this->begin() + this->size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::reverse_iterator string_literal<N>::rbegin() noexcept {
	return m_value.rbegin() + this->unused_size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reverse_iterator string_literal<N>::rbegin() const noexcept {
	return m_value.rbegin() + this->unused_size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reverse_iterator string_literal<N>::rcbegin() const noexcept {
	return m_value.rcbegin() + this->unused_size();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::reverse_iterator string_literal<N>::rend() noexcept {
	return m_value.rend();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reverse_iterator string_literal<N>::rend() const noexcept {
	return m_value.rend();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_reverse_iterator string_literal<N>::rcend() const noexcept {
	return m_value.crend();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic bool string_literal<N>::empty() const noexcept {
	return this->front() == '\0';
}


//--------------[ string interface ]--------------//

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::pointer string_literal<N>::c_str() noexcept {
	return m_value.data();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::const_pointer string_literal<N>::c_str() const noexcept {
	return m_value.data();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic std::string_view string_literal<N>::view() const {
	return { this->c_str() };
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::find(char c, ssize_t pos) const {
	const auto m_length = this->length();
	ZTU_ASSERT(0 <= pos and pos <= m_length, std::out_of_range("Given start pos is out of range."));
	return std::find(this->begin() + pos, this->begin() + m_length, c) - this->begin();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::find(const char *str, size_type len, ssize_t pos) const {
	const auto m_length = this->length();
	ZTU_ASSERT(0 <= pos and pos <= m_length, std::out_of_range("Given start pos is out of range."));
	return std::search(this->begin() + pos, this->begin() + m_length, str, str + len) - this->begin();
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::find(const char *str, ssize_t pos) const {
	return this->find(str, detail::strlen(str), pos);
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::find(const std::string_view &str, ssize_t pos) const {
	return this->find(str.data(), str.length(), pos);
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic string_literal<N>::size_type string_literal<N>::find(const std::string &str, ssize_t pos) const {
	return this->find(str.data(), str.length(), pos);
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_ic string_literal<N>::size_type string_literal<N>::find(const string_literal<M> &str) const {
	return this->find(str.c_str(), str.length());
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::resize(size_type new_size, char fill) {
	ZTU_ASSERT(0 <= new_size and new_size <= max_size, std::length_error("New size exceeds capacity."));
	m_value[new_size] = '\0';
	if (const auto m_size = this->size(); new_size > m_size)
		std::fill_n(this->begin() + m_size, new_size - m_size, fill);
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::erase(const_iterator begin_it, const_iterator end_it) {
	const auto m_size = this->size();
	const auto m_end = this->begin() + m_size;
	ZTU_ASSERT(this->begin() <= begin_it and begin_it <= m_end, std::out_of_range("begin iterator out of range"));
	ZTU_ASSERT(this->begin() <= end_it   and end_it   <= m_end, std::out_of_range("end iterator out of range"));
	auto mutable_begin = this->begin() + (begin_it - this->cbegin());
	auto mutable_end   = this->begin() + (end_it   - this->cbegin());
	const auto right_begin = mutable_end;
	const auto right_end = this->begin() + m_size + 1;
	std::copy(right_begin, right_end, mutable_begin);
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::erase(size_type index, size_type count) {
	const auto begin_it = this->begin() + index;
	return this->erase(begin_it, begin_it + count);
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::insert(size_type index, char c, size_type count) {
	const auto m_size = this->size();
	ZTU_ASSERT(0 <= index and index <= m_size, std::out_of_range("given index is out of range"));
	ZTU_ASSERT(0 <= count and count <= (max_size - m_size), std::length_error("given sequence exceeds capacity"));
	// move right of index with terminator
	const auto right_begin = this->begin() + index;
	const auto right_end = this->begin() + m_size + 1;
	std::copy_backward(right_begin, right_end, right_end + count);
	std::fill_n(this->begin() + index, count, c);
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::insert(size_type index, const char *str, size_type count) {
	const auto m_size = this->size();
	ZTU_ASSERT(0 <= index and index <= m_size, std::out_of_range("given index is out of range"));
	ZTU_ASSERT(0 <= count and count <= (max_size - m_size), std::length_error("given sequence exceeds capacity"));
	// move right of index with terminator
	const auto right_begin = this->begin() + index;
	const auto right_end = this->begin() + m_size + 1;
	std::copy_backward(right_begin, right_end, right_end + count);
	std::copy_n(str, count, this->begin() + index);
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::insert(size_type index, const char *str) {
	return this->insert(index, str, detail::strlen(str));
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::insert(size_type index, const std::string_view &str) {
	return this->insert(index, str.data(), str.length());
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::insert(size_type index, const std::string &str) {
	return this->insert(index, str.data(), str.length());
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_ic string_literal<N>::this_type& string_literal<N>::insert(size_type index, const string_literal<M> &str) {
	return this->insert(index, str.data(), str.length());
}


template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::replace(size_type index, size_type count, char c, size_type repeat) {
	const auto m_size = this->size();
	ZTU_ASSERT(0 <= index and index + count <= m_size, std::out_of_range("given index is out of range"));
	ZTU_ASSERT(0 <= count and 0 <= repeat, std::out_of_range("count and repeat must be none negative"));
	const auto delta_size = repeat - count;
	ZTU_ASSERT((m_size + delta_size) <= max_size, std::length_error("given sequence exceeds capacity"));

	const auto right_begin = this->begin() + index + count;
	const auto right_end = this->begin() + m_size + 1;

	if (delta_size < 0) {
		std::copy(right_begin, right_end, right_begin + delta_size);
	} else if (delta_size > 0) {
		std::copy_backward(right_begin, right_end, right_end + delta_size);
	}
	std::fill_n(this->begin() + index, repeat, c);

	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::replace(size_type index, size_type count, const char *str, size_type len) {
	const auto m_size = this->size();
	ZTU_ASSERT(0 <= index and index + count <= m_size, std::out_of_range("given index is out of range"));
	ZTU_ASSERT(0 <= count and 0 <= len, std::out_of_range("count and len must be none negative"));
	const auto delta_size = len - count;
	ZTU_ASSERT((m_size + delta_size) <= max_size, std::length_error("given sequence exceeds capacity"));

	const auto right_begin = this->begin() + index + count;
	const auto right_end = this->begin() + m_size + 1;

	if (delta_size < 0) {
		std::copy(right_begin, right_end, right_begin + delta_size);
	} else if (delta_size > 0) {
		std::copy_backward(right_begin, right_end, right_end + delta_size);
	}
	std::copy_n(str, len, this->begin() + index);

	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::replace(size_type index, size_type count, const char *str) {
	return this->replace(index, count, str, detail::strlen(str));
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::replace(size_type index, size_type count, const std::string_view &str) {
	return this->replace(index, count, str.data(), str.length());
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::replace(size_type index, size_type count, const std::string &str) {
	return this->replace(index, count, str.data(), str.length());
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_ic string_literal<N>::this_type& string_literal<N>::replace(size_type index, size_type count, const string_literal<M> &str) {
	return this->replace(index, count, str.data(), str.length());
}


//--------------[ operators ]--------------//

template<sl_ssize_t N> requires (N > 0)
ztu_nic bool string_literal<N>::operator==(const std::string &str) const {
	const auto o_length = static_cast<sl_ssize_t>(str.length());
	for (size_type i = 0; i < o_length; i++) {
		if (m_value[i] == '\0' or m_value[i] != str[i]) {
			return false;
		}
	}
	return true;
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic bool string_literal<N>::operator==(const std::string_view &str) const {
	const auto o_length = static_cast<sl_ssize_t>(str.length());
	for (size_type i = 0; i < o_length; i++) {
		if (m_value[i] == '\0' or m_value[i] != str[i]) {
			return false;
		}
	}
	return true;
}

template<sl_ssize_t N> requires (N > 0)
ztu_nic bool string_literal<N>::operator==(const char* str) const {
	size_type i = 0;
	do {
		if (i == max_size) [[unlikely]]
			return true;
		if (m_value[i] != str[i])
			return false;
	} while (m_value[i++] != '\0');
	return true;
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_nic bool string_literal<N>::operator==(const string_literal<M> &str) const {
	return (*this) == str.c_str();
}

template<sl_ssize_t N> requires (N > 0)
template<bool KnownToFit>
ztu_ic void string_literal<N>::assign(const char* str, size_type len) {
	if constexpr (not KnownToFit) {
		ZTU_ASSERT(len <= max_size, std::length_error("given string exceeds capacity"));
	}
	std::copy_n(str, len, m_value.begin());
	m_value[len] = '\0';
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::operator=(const std::string &str) {
	assign(str.data(), str.length());
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::operator=(const std::string_view &str) {
	assign(str.data(), str.length());
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::operator=(const char* str) {
	assign(str, detail::strlen(str));
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_ic string_literal<N>::this_type& string_literal<N>::operator=(const string_literal<M> &str) {
	assign<true>(str.data(), str.length());
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic void string_literal<N>::append(const char* str, size_type len) {
	const auto m_length = this->length();
	ZTU_ASSERT(len <= (max_size - m_length), std::length_error("given string exceeds available capacity"));
	std::copy_n(str, len, this->begin() + m_length);
	m_value[m_length + len] = '\0';
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::operator+=(const std::string &str) {
	append(str.data(), str.length());
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::operator+=(const std::string_view &str) {
	append(str.data(), str.length());
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
ztu_ic string_literal<N>::this_type& string_literal<N>::operator+=(const char* str) {
	append(str, detail::strlen(str));
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_ic string_literal<N>::this_type& string_literal<N>::operator+=(const string_literal<M> &str) {
	append(str.data(), str.length());
	return *this;
}

template<sl_ssize_t N> requires (N > 0)
template<sl_ssize_t M>
ztu_nic string_literal<M + N - 1> string_literal<N>::operator+(const string_literal<M> &str) const {
	string_literal<N + M - 1> combined{};

	const auto m_length = this->length();
	const auto o_length = str.length();

	std::copy_n(this->begin(), m_length, combined.begin());
	std::copy_n(str.begin(), o_length + 1, combined.begin() + m_length); // copy termination

	return combined;
}

template<sl_ssize_t M>
inline std::ostream& operator<<(std::ostream &out, const string_literal<M>& str) {
	return out << str.c_str();
}

namespace string_literals {
template<string_literal Str>
constexpr auto operator"" _sl() {
	return Str;
}
} // string_literals

#undef ztu_ic 
#undef ztu_nic

} // ztu
