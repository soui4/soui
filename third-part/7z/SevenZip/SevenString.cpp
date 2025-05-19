
#include "SevenString.h"
#include <vector>

#define GHC_USE_WCHAR_T

namespace SevenZip
{
	namespace detail {

		enum utf8_states_t { S_STRT = 0, S_RJCT = 8 };

		template <class T>
		struct _is_basic_string : std::false_type
		{
		};
		template <class CharT, class Traits, class Alloc>
		struct _is_basic_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type
		{
		};
		template <class CharT>
		struct _is_basic_string<std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>> : std::true_type
		{
		};
#ifdef GHC_WITH_STRING_VIEW
		template <class CharT, class Traits>
		struct _is_basic_string<basic_string_view<CharT, Traits>> : std::true_type
		{
		};
		template <class CharT>
		struct _is_basic_string<basic_string_view<CharT, std::char_traits<CharT>>> : std::true_type
		{
		};
#endif

		inline bool in_range(uint32_t c, uint32_t lo, uint32_t hi)
		{
			return (static_cast<uint32_t>(c - lo) < (hi - lo + 1));
		}

		inline bool is_surrogate(uint32_t c)
		{
			return in_range(c, 0xd800, 0xdfff);
		}

		inline bool is_high_surrogate(uint32_t c)
		{
			return (c & 0xfffffc00) == 0xd800;
		}

		inline bool is_low_surrogate(uint32_t c)
		{
			return (c & 0xfffffc00) == 0xdc00;
		}

		inline void appendUTF8(std::string& str, uint32_t unicode)
		{
			if (unicode <= 0x7f) {
				str.push_back(static_cast<char>(unicode));
			}
			else if (unicode >= 0x80 && unicode <= 0x7ff) {
				str.push_back(static_cast<char>((unicode >> 6) + 192));
				str.push_back(static_cast<char>((unicode & 0x3f) + 128));
			}
			else if ((unicode >= 0x800 && unicode <= 0xd7ff) || (unicode >= 0xe000 && unicode <= 0xffff)) {
				str.push_back(static_cast<char>((unicode >> 12) + 224));
				str.push_back(static_cast<char>(((unicode & 0xfff) >> 6) + 128));
				str.push_back(static_cast<char>((unicode & 0x3f) + 128));
			}
			else if (unicode >= 0x10000 && unicode <= 0x10ffff) {
				str.push_back(static_cast<char>((unicode >> 18) + 240));
				str.push_back(static_cast<char>(((unicode & 0x3ffff) >> 12) + 128));
				str.push_back(static_cast<char>(((unicode & 0xfff) >> 6) + 128));
				str.push_back(static_cast<char>((unicode & 0x3f) + 128));
			}
			else {
#ifdef GHC_RAISE_UNICODE_ERRORS
				throw filesystem_error("Illegal code point for unicode character.", str, std::make_error_code(std::errc::illegal_byte_sequence));
#else
				appendUTF8(str, 0xfffd);
#endif
			}
		}

		// Thanks to Bjoern Hoehrmann (https://bjoern.hoehrmann.de/utf-8/decoder/dfa/)
		// and Taylor R Campbell for the ideas to this DFA approach of UTF-8 decoding;
		// Generating debugging and shrinking my own DFA from scratch was a day of fun!
		inline unsigned consumeUtf8Fragment(const unsigned state, const uint8_t fragment, uint32_t& codepoint)
		{
			static const uint32_t utf8_state_info[] = {
				// encoded states
				0x11111111u, 0x11111111u, 0x77777777u, 0x77777777u, 0x88888888u, 0x88888888u, 0x88888888u, 0x88888888u, 0x22222299u, 0x22222222u, 0x22222222u, 0x22222222u, 0x3333333au, 0x33433333u, 0x9995666bu, 0x99999999u,
				0x88888880u, 0x22818108u, 0x88888881u, 0x88888882u, 0x88888884u, 0x88888887u, 0x88888886u, 0x82218108u, 0x82281108u, 0x88888888u, 0x88888883u, 0x88888885u, 0u,          0u,          0u,          0u,
			};
			uint8_t category = fragment < 128 ? 0 : (utf8_state_info[(fragment >> 3) & 0xf] >> ((fragment & 7) << 2)) & 0xf;
			codepoint = (state ? (codepoint << 6) | (fragment & 0x3fu) : (0xffu >> category) & fragment);
			return state == S_RJCT ? static_cast<unsigned>(S_RJCT) : static_cast<unsigned>((utf8_state_info[category + 16] >> (state << 2)) & 0xf);
		}

		inline bool validUtf8(const std::string& utf8String)
		{
			std::string::const_iterator iter = utf8String.begin();
			unsigned utf8_state = S_STRT;
			std::uint32_t codepoint = 0;
			while (iter < utf8String.end()) {
				if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) == S_RJCT) {
					return false;
				}
			}
			if (utf8_state) {
				return false;
			}
			return true;
		}

		template <class StringType, class Utf8String, typename std::enable_if<detail::_is_basic_string<Utf8String>::value && (sizeof(typename Utf8String::value_type) == 1) && (sizeof(typename StringType::value_type) == 1)>::type* = nullptr>
		inline StringType fromUtf8(const Utf8String& utf8String, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
		{
			return StringType(utf8String.begin(), utf8String.end(), alloc);
		}

		template <class StringType, class Utf8String, typename std::enable_if<detail::_is_basic_string<Utf8String>::value && (sizeof(typename Utf8String::value_type) == 1) && (sizeof(typename StringType::value_type) == 2)>::type* = nullptr>
		inline StringType fromUtf8(const Utf8String& utf8String, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
		{
			StringType result(alloc);
			result.reserve(utf8String.length());
			auto iter = utf8String.cbegin();
			unsigned utf8_state = S_STRT;
			std::uint32_t codepoint = 0;
			while (iter < utf8String.cend()) {
				if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) == S_STRT) {
					if (codepoint <= 0xffff) {
						result += static_cast<typename StringType::value_type>(codepoint);
					}
					else {
						codepoint -= 0x10000;
						result += static_cast<typename StringType::value_type>((codepoint >> 10) + 0xd800);
						result += static_cast<typename StringType::value_type>((codepoint & 0x3ff) + 0xdc00);
					}
					codepoint = 0;
				}
				else if (utf8_state == S_RJCT) {
#ifdef GHC_RAISE_UNICODE_ERRORS
					throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
					result += static_cast<typename StringType::value_type>(0xfffd);
					utf8_state = S_STRT;
					codepoint = 0;
#endif
				}
			}
			if (utf8_state) {
#ifdef GHC_RAISE_UNICODE_ERRORS
				throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
				result += static_cast<typename StringType::value_type>(0xfffd);
#endif
			}
			return result;
		}

		template <class StringType, class Utf8String, typename std::enable_if<detail::_is_basic_string<Utf8String>::value && (sizeof(typename Utf8String::value_type) == 1) && (sizeof(typename StringType::value_type) == 4)>::type* = nullptr>
		inline StringType fromUtf8(const Utf8String& utf8String, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
		{
			StringType result(alloc);
			result.reserve(utf8String.length());
			auto iter = utf8String.cbegin();
			unsigned utf8_state = S_STRT;
			std::uint32_t codepoint = 0;
			while (iter < utf8String.cend()) {
				if ((utf8_state = consumeUtf8Fragment(utf8_state, static_cast<uint8_t>(*iter++), codepoint)) == S_STRT) {
					result += static_cast<typename StringType::value_type>(codepoint);
					codepoint = 0;
				}
				else if (utf8_state == S_RJCT) {
#ifdef GHC_RAISE_UNICODE_ERRORS
					throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
					result += static_cast<typename StringType::value_type>(0xfffd);
					utf8_state = S_STRT;
					codepoint = 0;
#endif
				}
			}
			if (utf8_state) {
#ifdef GHC_RAISE_UNICODE_ERRORS
				throw filesystem_error("Illegal byte sequence for unicode character.", utf8String, std::make_error_code(std::errc::illegal_byte_sequence));
#else
				result += static_cast<typename StringType::value_type>(0xfffd);
#endif
			}
			return result;
		}

		template <class StringType, typename charT, std::size_t N>
		inline StringType fromUtf8(const charT(&utf8String)[N])
		{
#ifdef GHC_WITH_STRING_VIEW
			return fromUtf8<StringType>(basic_string_view<charT>(utf8String, N - 1));
#else
			return fromUtf8<StringType>(std::basic_string<charT>(utf8String, N - 1));
#endif
		}

		template <typename strT, typename std::enable_if<detail::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 1), int>::type size = 1>
		inline std::string toUtf8(const strT& unicodeString)
		{
			return std::string(unicodeString.begin(), unicodeString.end());
		}

		template <typename strT, typename std::enable_if<detail::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 2), int>::type size = 2>
		inline std::string toUtf8(const strT& unicodeString)
		{
			std::string result;
			for (auto iter = unicodeString.begin(); iter != unicodeString.end(); ++iter) {
				char32_t c = *iter;
				if (is_surrogate(c)) {
					++iter;
					if (iter != unicodeString.end() && is_high_surrogate(c) && is_low_surrogate(*iter)) {
						appendUTF8(result, (char32_t(c) << 10) + *iter - 0x35fdc00);
					}
					else {
#ifdef GHC_RAISE_UNICODE_ERRORS
						throw filesystem_error("Illegal code point for unicode character.", result, std::make_error_code(std::errc::illegal_byte_sequence));
#else
						appendUTF8(result, 0xfffd);
						if (iter == unicodeString.end()) {
							break;
						}
#endif
					}
				}
				else {
					appendUTF8(result, c);
				}
			}
			return result;
		}

		template <typename strT, typename std::enable_if<detail::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 4), int>::type size = 4>
		inline std::string toUtf8(const strT& unicodeString)
		{
			std::string result;
			for (auto c : unicodeString) {
				appendUTF8(result, static_cast<uint32_t>(c));
			}
			return result;
		}

		template <typename charT>
		inline std::string toUtf8(const charT* unicodeString)
		{
#ifdef GHC_WITH_STRING_VIEW
			return toUtf8(basic_string_view<charT, std::char_traits<charT>>(unicodeString));
#else
			return toUtf8(std::basic_string<charT, std::char_traits<charT>>(unicodeString));
#endif
		}

#ifdef GHC_USE_WCHAR_T
		template <class StringType, class WString, typename std::enable_if<detail::_is_basic_string<WString>::value && (sizeof(typename WString::value_type) == 2) && (sizeof(typename StringType::value_type) == 1), bool>::type = false>
		inline StringType fromWChar(const WString& wString, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
		{
			auto temp = toUtf8(wString);
			return StringType(temp.begin(), temp.end(), alloc);
		}

		template <class StringType, class WString, typename std::enable_if<detail::_is_basic_string<WString>::value && (sizeof(typename WString::value_type) == 2) && (sizeof(typename StringType::value_type) == 2), bool>::type = false>
		inline StringType fromWChar(const WString& wString, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
		{
			return StringType(wString.begin(), wString.end(), alloc);
		}

		template <class StringType, class WString, typename std::enable_if<detail::_is_basic_string<WString>::value && (sizeof(typename WString::value_type) == 2) && (sizeof(typename StringType::value_type) == 4), bool>::type = false>
		inline StringType fromWChar(const WString& wString, const typename StringType::allocator_type& alloc = typename StringType::allocator_type())
		{
			auto temp = toUtf8(wString);
			return fromUtf8<StringType>(temp, alloc);
		}

		template <typename strT, typename std::enable_if<detail::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 1), bool>::type = false>
		inline std::wstring toWChar(const strT& unicodeString)
		{
			return fromUtf8<std::wstring>(unicodeString);
		}

		template <typename strT, typename std::enable_if<detail::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 2), bool>::type = false>
		inline std::wstring toWChar(const strT& unicodeString)
		{
			return std::wstring(unicodeString.begin(), unicodeString.end());
		}

		template <typename strT, typename std::enable_if<detail::_is_basic_string<strT>::value && (sizeof(typename strT::value_type) == 4), bool>::type = false>
		inline std::wstring toWChar(const strT& unicodeString)
		{
			auto temp = toUtf8(unicodeString);
			return fromUtf8<std::wstring>(temp);
		}

		template <typename charT>
		inline std::wstring toWChar(const charT* unicodeString)
		{
#ifdef GHC_WITH_STRING_VIEW
			return toWChar(basic_string_view<charT, std::char_traits<charT>>(unicodeString));
#else
			return toWChar(std::basic_string<charT, std::char_traits<charT>>(unicodeString));
#endif
		}
#endif  // GHC_USE_WCHAR_T
	}

	std::string ToPathNativeStr(const std::wstring& str)
	{
#ifdef _WIN32
		std::string buf;
		if (str.empty()) {
			return buf;
		}

		if (!str.size()) {
			return buf;
		}

		// figure out how many narrow characters we are going to get
		int nChars = WideCharToMultiByte(CP_ACP, 0,
			str.c_str(), str.size(), NULL, 0, NULL, NULL);
		if (str.size() == -1)
			--nChars;
		if (nChars == 0)
			return "";

		// convert the wide string to a narrow string
		// nb: slightly naughty to write directly into the string like this
		buf.resize(nChars);
		WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(),
			const_cast<char*>(buf.c_str()), nChars, NULL, NULL);
		return buf;
#else
		//std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		//return convert.to_bytes(str);
		return detail::toUtf8<std::wstring>(str);
#endif	
	}

	std::string ToString(const std::wstring& wstring, const std::locale& locale)
	{
		using codecvt_t = std::codecvt<wchar_t, char, std::mbstate_t>;

		const codecvt_t& codecvt(std::use_facet<codecvt_t>(locale));
		std::mbstate_t state;
		std::vector<char> buffer((wstring.size() + 1) * codecvt.max_length());
		const wchar_t* in_next{ wstring.c_str() };
		char* out_next{ &buffer[0] };

		std::codecvt_base::result result{ codecvt.out(state, wstring.c_str(), wstring.c_str() + wstring.size(), in_next, &buffer[0], &buffer[0] + buffer.size(), out_next) };

		return &buffer[0];
	}

	std::wstring ToWstring(const std::string& string, const std::locale& locale)
	{
		using codecvt_t = std::codecvt<wchar_t, char, std::mbstate_t>;

		const codecvt_t& codecvt(std::use_facet<codecvt_t>(locale));
		std::mbstate_t state{};
		std::vector<wchar_t> buffer(string.size() + 1);
		const char* in_next{ string.c_str() };
		wchar_t* out_next{ &buffer[0] };

		std::codecvt_base::result result{ codecvt.in(state, string.c_str(), string.c_str() + string.size(), in_next, &buffer[0], &buffer[0] + buffer.size(), out_next) };

		return &buffer[0];
	}

	std::string ToU8string(const std::wstring &wstring, const std::locale& locale)
	{
		//std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		//return convert.to_bytes(wstring);
		//return detail::fromUtf8<std::wstring>(wstring)
		return detail::toUtf8<std::wstring>(wstring);
	}
}