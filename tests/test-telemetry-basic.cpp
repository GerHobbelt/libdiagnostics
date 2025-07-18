
#pragma once

#include <diagnostics/telemetry.h>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <format>
#include <iostream>
#include <chrono>




// ---------------------------------------------------------------
// https://www.cppstories.com/2022/custom-stdformat-cpp20/


struct Index {
	unsigned int id_{0};
};

// And then we can write the following formatter:

template <>
struct std::formatter<Index> {
	// for debugging only
	formatter() { std::cout << "formatter<Index>()\n"; }

	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Index& id, std::format_context& ctx) const {
		return std::format_to(ctx.out(), "{}", id.id_);
	}
};

// Use case:

	Index id{100};
	std::cout << std::format("id {}\n", id);
	std::cout << std::format("id duplicated {0} {0}\n", id);

	We have the following output:

	formatter<Index>()
		id 100
		formatter<Index>()
		formatter<Index>()
		id duplicated 100 100
		As you can see, even for a duplicated argument{0}, two formatters are created, not one.

		The parse() function takes the context and gets the format spec for a given argument.

		For example:

	"{0}"      // ctx.begin() points to `}`
		"{0:d}"    // ctx.begin() points to `d`, begin-end is "d}"
		"{:hello}" // ctx.begin points to 'h' and begin-end is "hello}"
		The parse() function has to return the iterator to the closing bracket, so we need to find it or assume it’s at the position of ctx.begin().

		In a case of{:hello} returning begin() will not point to } and thus, you’ll get some runtime error - an exception will be thrown. So be careful!

		For a simple case with just one value we can rely on the standard implementation and reuse it:

	template <>
	struct std::formatter<Index>: std::formatter<int> {
		auto format(const Index& id, std::format_context& ctx) const {
			return std::formatter<int>::format(id.id_, ctx);
		}
	};
	Now, our code will work and parse standard specifiers:

	Index id{100};
	std::cout << std::format("id {:*<11d}\n", id);
	std::cout << std::format("id {:*^11d}\n", id);
output:

	id 100********
		id ****100****
		Multiple Values
		How about cases where we’d like to show multiple values:

	struct Color {
		uint8_t r{0};
		uint8_t g{0};
		uint8_t b{0};
	};
	To create a formatter, we can use the following code:

	template <>
	struct std::formatter<Color> {
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		auto format(const Color& col, std::format_context& ctx) const {
			return std::format_to(ctx.out(), "({}, {}, {})", col.r, col.g, col.b);
		}
	};
	This supports only fixed output format and no additional format specifiers.

		We can, however rely on the predefined string_view formatter:

	template <>
	struct std::formatter<Color>: std::formatter<string_view> {
		auto format(const Color& col, std::format_context& ctx) const {
			std::string temp;
			std::format_to(std::back_inserter(temp), "({}, {}, {})",
						   col.r, col.g, col.b);
			return std::formatter<string_view>::format(temp, ctx);
		}
	};
	We don’t have to implement the parse() function with the above code. Inside format(), we output the color values to a temporary buffer, and then we reuse the underlying formatter to output the final string.

		Similarly, if your object holds a container of values, you can write the following code:

	template <>
	struct std::formatter<YourType>: std::formatter<string_view> {
		auto format(const YourType& obj, std::format_context& ctx) const {
			std::string temp;
			std::format_to(std::back_inserter(temp), "{} - ", obj.GetName());

			for (const auto& elem : obj.GetValues())
				std::format_to(std::back_inserter(temp), "{}, ", elem);

			return std::formatter<string_view>::format(temp, ctx);
		}
	};
	The formatter above will print obj.GetName() and then followed by elements from the obj.GetValues() container. Since we inherit from the string_view formatter class, the standard format specifiers also apply here.

		The format() function should be const
		Thanks to a watchful reader, I improved my article regarding the constness of the format() function.

		When I published the article in July 2022, it was unclear from the specification if the function should be const or not.

		But there was a fix, applied in November 2022, which corrected this wording:

	Issue 3636: formatter::format should be const-qualified

		According to Victor Zverovich, his intent was that f.parse(pc) should modify the state of f, but f.format(u, fc) should merely read f’s state to support format string compilation where formatter objects are immutable and therefore the format function must be const-qualified.

		Extending the formatter with parse() function
		But how about a custom parsing function?

		The main idea is that we can parse the format string and then store some state in *this, then we can use the information in the format call.

		Let’s try:

		template <>
	struct std::formatter<Color> {
		constexpr auto parse(std::format_parse_context& ctx) {
			auto pos = ctx.begin();
			while (pos != ctx.end() && *pos != '}') {
				if (*pos == 'h' || *pos == 'H')
					isHex_ = true;
				++pos;
			}
			return pos;  // expect `}` at this position, otherwise, 
			// it's error! exception!
		}

		auto format(const Color& col, std::format_context& ctx) const {
			if (isHex_) {
				uint32_t val = col.r << 16 | col.g << 8 | col.b;
				return std::format_to(ctx.out(), "#{:x}", val);
			}

			return std::format_to(ctx.out(), "({}, {}, {})", col.r, col.g, col.b);
		}

		bool isHex_{false};
	};
	And the test:

	std::cout << std::format("col {}\n", Color{100, 200, 255});
	std::cout << std::format("col {:h}\n", Color{100, 200, 255});
	the output:

	col (100, 200, 255)
		col #64c8ff


