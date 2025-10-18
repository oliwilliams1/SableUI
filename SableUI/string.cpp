#include "SableUI/string.h"
#include "SableUI/console.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

using namespace SableUI;

String::String() : m_data(nullptr), m_size(0) {}

String::~String()
{
	clear();
}

void String::clear() noexcept
{
	if (m_data)
	{
		delete[] m_data;
		m_data = nullptr;
		m_size = 0;
	}
}

String::String(const String& other) : m_data(nullptr), m_size(0)
{
	if (other.m_data && other.m_size > 0)
	{
		m_size = other.m_size;
		m_data = new char32_t[m_size + 1];
		std::copy(other.m_data, other.m_data + m_size + 1, m_data);
	}
}

String::String(String&& other) noexcept
	: m_data(other.m_data), m_size(other.m_size)
{
	other.m_data = nullptr;
	other.m_size = 0;
}

String::String(const char32_t* str) : m_data(nullptr), m_size(0)
{
	if (str)
	{
		size_t len = 0;
		while (str[len] != U'\0') len++;
		m_size = len;

		m_data = new char32_t[m_size + 1];
		std::copy(str, str + m_size + 1, m_data);
	}
}

String::String(const char* str) : m_data(nullptr), m_size(0)
{
	if (str)
	{
		size_t len = std::strlen(str);
		m_size = len;
		m_data = new char32_t[m_size + 1];
		for (size_t i = 0; i < m_size; ++i)
			m_data[i] = static_cast<char32_t>(str[i]);
		m_data[m_size] = U'\0';
	}
}

String& String::operator=(const String& other)
{
	if (this == &other) return *this;

	String temp(other);
	std::swap(m_data, temp.m_data);
	std::swap(m_size, temp.m_size);

	return *this;
}

String& String::operator=(String&& other) noexcept
{
	if (this != &other)
	{
		clear();
		m_data = other.m_data;
		m_size = other.m_size;

		other.m_data = nullptr;
		other.m_size = 0;
	}
	return *this;
}

String String::operator+(const String& other) const
{
	if (!m_data && !other.m_data)
		return String();

	String result;
	result.m_size = m_size + other.m_size;
	result.m_data = new char32_t[result.m_size + 1];

	if (m_data)
		std::copy(m_data, m_data + m_size, result.m_data);
	if (other.m_data)
		std::copy(other.m_data, other.m_data + other.m_size, result.m_data + m_size);

	result.m_data[result.m_size] = U'\0';
	return result;
}

bool String::operator==(const String& other) const
{
	if (m_size != other.m_size)
		return false;

	if (!m_data || !other.m_data)
		return m_data == other.m_data;

	return std::equal(m_data, m_data + m_size, other.m_data);
}

String::operator std::string() const
{
	if (!m_data)
		return {};

	std::string result;
	result.reserve(m_size);

	for (size_t i = 0; i < m_size; ++i)
	{
		result.push_back(static_cast<char>(m_data[i]));
	}
	return result;
}
