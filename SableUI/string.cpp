#include <SableUI/string.h>

#include <algorithm>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <string>

using namespace SableUI;

static int s_numComponents = 0;

String::StringData::StringData(const char32_t* str, size_t len)
	: size(len), refCount(1), data(nullptr)
{
	data = new char32_t[len + 1];
	if (str && len > 0)
		std::copy(str, str + len, data);

	data[len] = U'\0';
}

String::StringData::~StringData()
{
	delete[] data;
}

void String::AddRef()
{
	if (m_data)
		m_data->refCount++;
}

void String::Release()
{
	if (m_data)
	{
		m_data->refCount--;
		if (m_data->refCount == 0)
		{
			delete m_data;
			m_data = nullptr;
		}
	}
}

void String::MakeUnique()
{
	if (m_data && m_data->refCount > 1)
	{
		StringData* newData = new StringData(m_data->data, m_data->size);
		Release();
		m_data = newData;
	}
}

String::String() noexcept
	: m_data(nullptr), m_size(0)
{
	s_numComponents++;
}

String::~String() noexcept
{
	s_numComponents--;
	Release();
}

String::String(const String& other)
	: m_data(other.m_data), m_size(other.m_size)
{
	s_numComponents++;
	AddRef();
}

String::String(String&& other) noexcept
	: m_data(other.m_data), m_size(other.m_size)
{
	if (this != &other) s_numComponents++;
	other.m_data = nullptr;
	other.m_size = 0;
}

String::String(const char32_t* str)
	: m_data(nullptr), m_size(0)
{
	s_numComponents++;
	if (str)
	{
		size_t len = 0;
		while (str[len] != U'\0') len++;
		m_size = len;

		if (m_size > 0)
		{
			m_data = new StringData(str, m_size);
		}
	}
}

String::String(const char* str)
	: m_data(nullptr), m_size(0)
{
	s_numComponents++;
	if (str)
	{
		size_t len = std::strlen(str);
		m_size = len;

		if (m_size > 0)
		{
			m_data = new StringData(nullptr, m_size);
			for (size_t i = 0; i < m_size; ++i)
				m_data->data[i] = static_cast<char32_t>(str[i]);
			m_data->data[m_size] = U'\0';
		}
	}
}

String::String(const std::string& str)
	: m_data(nullptr), m_size(0)
{
	s_numComponents++;
	m_size = str.size();

	if (m_size > 0)
	{
		m_data = new StringData(nullptr, m_size);
		for (size_t i = 0; i < m_size; ++i)
			m_data->data[i] = static_cast<char32_t>(str[i]);
		m_data->data[m_size] = U'\0';
	}
}

String& String::operator=(const String& other)
{
	if (this == &other) return *this;

	Release();
	m_data = other.m_data;
	m_size = other.m_size;
	AddRef();

	return *this;
}

String& String::operator=(String&& other) noexcept
{
	if (this != &other)
	{
		Release();
		m_data = other.m_data;
		m_size = other.m_size;
		other.m_data = nullptr;
		other.m_size = 0;
	}
	return *this;
}

void String::clear() noexcept
{
	Release();
	m_size = 0;
}

int String::GetNumInstances()
{
	return s_numComponents;
}

String String::wrap(StyleTag start, StyleTag end) const
{
	String result;
	result.m_size = m_size + 2;
	result.m_data = new StringData(nullptr, result.m_size);

	result.m_data->data[0] = static_cast<char32_t>(start);
	if (m_data && m_data->data)
	{
		std::copy(m_data->data, m_data->data + m_size, result.m_data->data + 1);
	}
	result.m_data->data[result.m_size - 1] = static_cast<char32_t>(end);
	result.m_data->data[result.m_size] = U'\0';

	return result;
}

String String::bold() const { return wrap(StyleTag::BoldStart, StyleTag::BoldEnd); }
String String::italic() const { return wrap(StyleTag::ItalicStart, StyleTag::ItalicEnd); }
String String::boldItalic() const { return wrap(StyleTag::BoldItalicStart, StyleTag::BoldItalicEnd); }
String String::light() const { return wrap(StyleTag::LightStart, StyleTag::LightEnd); }
String String::lightItalic() const { return wrap(StyleTag::LightItalicStart, StyleTag::LightItalicEnd); }

String String::Format(const char* format, ...)
{
	String result;

	va_list args_copy;
	va_list args;
	va_start(args, format);
	va_copy(args_copy, args);
	int narrow_size = std::vsnprintf(nullptr, 0, format, args_copy);
	va_end(args_copy);

	if (narrow_size < 0)
	{
		va_end(args);
		return String();
	}

	std::string narrow_buffer(narrow_size, '\0');
	std::vsnprintf(narrow_buffer.data(), static_cast<size_t>(narrow_size) + 1, format, args);
	va_end(args);
	result = String(narrow_buffer);

	return result;
}

String String::operator+(const String& other) const
{
	if (!m_data && !other.m_data)
		return String();

	String result;
	result.m_size = m_size + other.m_size;
	result.m_data = new StringData(nullptr, result.m_size);

	if (m_data && m_data->data)
		std::copy(m_data->data, m_data->data + m_size, result.m_data->data);
	if (other.m_data && other.m_data->data)
		std::copy(other.m_data->data, other.m_data->data + other.m_size, result.m_data->data + m_size);

	result.m_data->data[result.m_size] = U'\0';
	return result;
}

bool String::operator==(const String& other) const
{
	if (m_size != other.m_size)
		return false;

	if (!m_data && !other.m_data)
		return true;

	if (!m_data || !other.m_data)
		return false;

	return std::equal(m_data->data, m_data->data + m_size, other.m_data->data);
}

String::operator std::string() const
{
	if (!m_data || !m_data->data)
		return {};

	std::string result;
	result.reserve(m_size);

	for (size_t i = 0; i < m_size; ++i)
	{
		result.push_back(static_cast<char>(m_data->data[i]));
	}
	return result;
}

void String::push_back(char32_t c)
{
	MakeUnique();

	size_t oldSize = m_size;
	size_t newSize = oldSize + 1;

	StringData* newData = new StringData(nullptr, newSize);

	if (m_data && m_data->data)
		std::copy(m_data->data, m_data->data + oldSize, newData->data);

	newData->data[oldSize] = c;
	newData->data[newSize] = U'\0';

	Release();
	m_data = newData;
	m_size = newSize;
}

String String::operator+(const char32_t other) const
{
	String result;
	result.m_size = m_size + 1;
	result.m_data = new StringData(nullptr, result.m_size);

	if (m_data && m_data->data)
		std::copy(m_data->data, m_data->data + m_size, result.m_data->data);

	result.m_data->data[m_size] = other;
	result.m_data->data[result.m_size] = U'\0';

	return result;
}

String String::operator+(const char other) const
{
	return *this + static_cast<char32_t>(other);
}

String String::substr(size_t pos, size_t count) const
{
	if (!m_data || !m_data->data)
		return String();

	if (pos > m_size)
		pos = m_size;

	if (count == size_t(-1) || pos + count > m_size)
		count = m_size - pos;

	String result;
	result.m_size = count;
	result.m_data = new StringData(m_data->data + pos, count);
	return result;
}

size_t String::size() const noexcept
{
	return m_size;
}

bool String::empty() const noexcept
{
	return m_size == 0;
}

char32_t String::operator[](size_t index) const noexcept
{
	if (!m_data || !m_data->data || index >= m_size)
		return U'\0';
	return m_data->data[index];
}

String::iterator String::begin() noexcept
{
	MakeUnique();
	return m_data ? m_data->data : nullptr;
}

String::iterator String::end() noexcept
{
	MakeUnique();
	return m_data ? (m_data->data + m_size) : nullptr;
}

String::const_iterator String::begin() const noexcept
{
	return m_data ? m_data->data : nullptr;
}

String::const_iterator String::end() const noexcept
{
	return m_data ? (m_data->data + m_size) : nullptr;
}