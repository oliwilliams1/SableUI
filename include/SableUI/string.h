#pragma once
#include <string>
#include <cstddef>
#include <utility>

namespace SableUI
{
	class String
	{
	public:
		String() noexcept;
		~String() noexcept;

		String(const String& other);
		String(String&& other) noexcept;
		String(const char32_t* str);
		String(const char* str);
		String(const std::string& str);

		static int GetNumInstances();

		String& operator=(const String& other);
		String& operator=(String&& other) noexcept;
		String operator+(const String& other) const;
		bool operator==(const String& other) const;

		size_t size() const noexcept { return m_size; }
		bool empty() const noexcept { return m_size == 0; }
		char32_t operator[](size_t index) const noexcept { return m_data[index]; }

		operator std::string() const;

		using iterator = char32_t*;
		using const_iterator = const char32_t*;

		iterator begin() noexcept { return m_data; }
		iterator end() noexcept { return m_data + m_size; }
		const_iterator begin() const noexcept { return m_data; }
		const_iterator end() const noexcept { return m_data + m_size; }

	private:
		void clear() noexcept;

		char32_t* m_data;
		size_t m_size;
	};
}
