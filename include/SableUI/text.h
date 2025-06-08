#pragma once
#include <string>

namespace SableUI
{
	struct Text
	{
		Text() = default;
		~Text() {};

		Text(const Text&) = delete;
		Text& operator=(const Text&) = delete;

		void SetContent(const std::u32string& str);

	private:
		std::u32string content = U"";
	};
}