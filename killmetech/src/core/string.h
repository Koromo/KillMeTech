#ifndef _KILLME_STRING_H_
#define _KILLME_STRING_H_

#include <tchar.h>
#include <string>
#include <cstdarg>

// Convert to used character set by application
#define KILLME_T(s) _TEXT(s)

namespace killme
{
    /** Convert to multibyte string */
    std::string narrow(const std::string& s);
    std::string narrow(const std::wstring& s);

    /** Convert to wide string */
    std::wstring widen(const std::string& s);
    std::wstring widen(const std::wstring& s);

    /** Convert to lowers string */
    std::string toLowers(const std::string& s);
    std::wstring toLowers(const std::wstring& s);

    /** Convert to uppers string */
    std::string toUppers(const std::string& s);
    std::wstring toUppers(const std::wstring& s);

    /** Compare strings with lowers */
    int strcmpLow(const std::string& a, const std::string& b);
    int strcmpLow(const std::wstring& a, const std::wstring& b);

    /** Returns length */
    size_t strlen(const std::string& s);
    size_t strlen(const std::wstring& s);

    /** Store format stirng */
    int vsprintf(char* buffer, const char* fmt, va_list args);
    int vsprintf(wchar_t* buffer, const wchar_t* fmt, va_list args);

    // "t" or "T" prefix denote used character set by application
    using tchar = TCHAR;
    using tstring = std::basic_string<tchar, std::char_traits<tchar>, std::allocator<tchar>>;

    // Convert to used character set by application
    tstring toCharSet(const std::string& s);
    tstring toCharSet(const std::wstring& s);
}

#endif