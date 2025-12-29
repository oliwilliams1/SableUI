#pragma once
#include <type_traits>
#include <string>

namespace SableUI
{
	enum class StyleTag : char32_t
	{
		BoldStart = U'\uE000',
		BoldEnd = U'\uE001',
		ItalicStart = U'\uE002',
		ItalicEnd = U'\uE003',
		BoldItalicStart = U'\uE004',
		BoldItalicEnd = U'\uE005',
		LightStart = U'\uE006',
		LightEnd = U'\uE007',
		LightItalicStart = U'\uE008',
		LightItalicEnd = U'\uE009'
	};

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
		String(const std::u32string& str);

		String wrap(StyleTag startTag, StyleTag endTag) const;
		String bold() const;
		String italic() const;
		String boldItalic() const;
		String light() const;
		String lightItalic() const;
		static String Format(const char* format, ...);

		static int GetNumInstances();

		String& operator=(const String& other);
		String& operator=(String&& other) noexcept;
		String operator+(const String& other) const;
		bool operator==(const String& other) const;
		String operator+(const char32_t other) const;
		String operator+(const char other) const;

		String substr(size_t pos, size_t count = static_cast<size_t>(-1)) const;

		size_t size() const noexcept;
		bool empty() const noexcept;
		char32_t operator[](size_t index) const noexcept;

		operator std::string() const;

		using iterator = char32_t*;
		using const_iterator = const char32_t*;
		void push_back(char32_t);

		iterator begin() noexcept;
		iterator end() noexcept;
		const_iterator begin() const noexcept;
		const_iterator end() const noexcept;

		void clear() noexcept;

	private:
		struct StringData
		{
			char32_t* data = nullptr;
			size_t size = 0;
			size_t refCount = 1;

			StringData() = default;
			StringData(const char32_t* str, size_t len);
			~StringData();
		};

		StringData* m_data;
		size_t m_size; // Keep for quick access

		void AddRef();
		void Release();
		void MakeUnique();
	};

	inline String operator+(const char32_t* lhs, const String& rhs)
	{
		String left(lhs);
		return left + rhs;
	}

	inline String operator+(const char* lhs, const String& rhs)
	{
		String left(lhs);
		return left + rhs;
	}
}

namespace std
{
	template<>
	struct hash<SableUI::String>
	{
		std::size_t operator()(const SableUI::String& s) const noexcept
		{
			std::size_t h = 0;
			std::hash<char32_t> charHasher;

			for (auto c : s)
			{
				std::size_t charHash = charHasher(c);
				h ^= charHash + 0x9e3779b9 + (h << 6) + (h >> 2);
			}
			return h;
		}
	};
}