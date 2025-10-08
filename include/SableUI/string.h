#pragma once
#include <string>

namespace SableUI
{
	class String
	{
	public:
		String();
		~String();
		String(const String& other);
		String(const char32_t* str);
		String(const char* str);

		// Accessors
		size_t size() const;
		char32_t operator[](size_t index) const;

		operator std::string() const;

		// Operators
		String(SableUI::String&& other) noexcept;
		String operator+(const String& other) const;
		String& operator=(const String& other);
		String& operator=(SableUI::String&& other) noexcept;
		bool operator==(const String& other) const;

		using iterator = char32_t*;
		using const_iterator = const char32_t*;

		iterator begin();
		iterator end();

		const_iterator begin() const;
		const_iterator end() const;

	private:
		char32_t* m_data = nullptr;
		size_t m_size = 0;
	};
}
