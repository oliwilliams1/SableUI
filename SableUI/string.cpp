#include "SableUI/string.h"
#include <algorithm>

SableUI::String::String() : m_data(nullptr), m_size(0) {}

SableUI::String::~String() { if (m_data) delete[] m_data; }

SableUI::String::String(const SableUI::String& other) : m_data(nullptr), m_size(0)
{
	if (other.m_data)
	{
		m_size = other.m_size;
		m_data = new char32_t[m_size + 1];
		std::copy(other.m_data, other.m_data + m_size + 1, m_data);
	}
}

SableUI::String::String(SableUI::String&& other) noexcept
	: m_data(other.m_data), m_size(other.m_size)
{
	other.m_data = nullptr;
	other.m_size = 0;
}


SableUI::String::String(const char32_t* str)
{
	if (str)
	{
		size_t len = 0;
		while (str[len] != U'\0') len++;
		m_size = len;
		m_data = new char32_t[m_size + 1];
		std::copy(str, str + m_size + 1, m_data);
	}
	else
	{
		m_data = nullptr;
		m_size = 0;
	}
}

SableUI::String::String(const char* str)
{
	if (str)
	{
		size_t len = 0;
		while (str[len] != '\0') len++;
		m_size = len;
		m_data = new char32_t[m_size + 1];
		for (size_t i = 0; i < m_size; i++)
		{
			m_data[i] = static_cast<char32_t>(str[i]);
		}
		m_data[m_size] = U'\0';
	}
	else {
		m_data = nullptr;
		m_size = 0;
	}
}

SableUI::String& SableUI::String::operator=(SableUI::String&& other) noexcept
{
	if (this != &other)
	{
		delete[] m_data;

		m_data = other.m_data;
		m_size = other.m_size;

		other.m_data = nullptr;
		other.m_size = 0;
	}
	return *this;
}

SableUI::String& SableUI::String::operator=(const SableUI::String& other)
{
	if (this != &other)
	{
		delete[] m_data;
		m_data = nullptr;
		m_size = 0;

		if (other.m_data)
		{
			m_size = other.m_size;
			m_data = new char32_t[m_size + 1];
			std::copy(other.m_data, other.m_data + m_size + 1, m_data);
		}
	}
	return *this;
}

SableUI::String SableUI::String::operator+(const SableUI::String& other) const
{
	String result;
	result.m_size = m_size + other.m_size;
	result.m_data = new char32_t[result.m_size + 1];

	std::copy(m_data, m_data + m_size, result.m_data);
	std::copy(other.m_data, other.m_data + other.m_size, result.m_data + m_size);
	result.m_data[result.m_size] = U'\0';

	return result;
}

bool SableUI::String::operator==(const SableUI::String& other) const
{
	if (m_size != other.m_size) return false;
	return std::equal(m_data, m_data + m_size, other.m_data);
}

size_t SableUI::String::size() const { return m_size; }

char32_t SableUI::String::operator[](size_t index) const { return m_data[index]; }

SableUI::String::operator std::string() const
{
	std::string result;
	result.resize(m_size);
	for (size_t i = 0; i < m_size; i++)
	{
		result[i] = static_cast<char>(m_data[i]);
	}
	return result;
}

SableUI::String::iterator SableUI::String::begin() { return m_data; }
SableUI::String::iterator SableUI::String::end() { return m_data + m_size; }

SableUI::String::const_iterator SableUI::String::begin() const { return m_data; }
SableUI::String::const_iterator SableUI::String::end() const { return m_data + m_size; }
