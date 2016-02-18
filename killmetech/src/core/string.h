#ifndef _KILLME_STRING_H_
#define _KILLME_STRING_H_

#include <tchar.h>
#include <string>
#include <cstdarg>

/** Convert to the used character set */
#define KILLME_T(s) _TEXT(s)

namespace killme
{
    /** Convert to the multibyte string */
    std::string narrow(const std::string& s);
    std::string narrow(const std::wstring& s);

    /** Convert to the wide string */
    std::wstring widen(const std::string& s);
    std::wstring widen(const std::wstring& s);

    /** Convert to the lowers string */
    std::string toLowers(const std::string& s);
    std::wstring toLowers(const std::wstring& s);

    /** Convert to the uppers string */
    std::string toUppers(const std::string& s);
    std::wstring toUppers(const std::wstring& s);

    /** Compare strings with lowers */
    int strcmpLow(const std::string& a, const std::string& b);
    int strcmpLow(const std::wstring& a, const std::wstring& b);

    /** Returns the length */
    size_t strlen(const std::string& s);
    size_t strlen(const std::wstring& s);

    /** Store the format stirng */
    int vsprintf(char* buffer, const char* fmt, va_list args);
    int vsprintf(wchar_t* buffer, const wchar_t* fmt, va_list args);

    /** The character type definitions that are fixed to the character set */
    using tchar = TCHAR;
    using tstring = std::basic_string<tchar, std::char_traits<tchar>, std::allocator<tchar>>;

    // Convert to the used character set
    tstring toCharSet(const std::string& s);
    tstring toCharSet(const std::wstring& s);
}

#endif