#ifndef _KILLME_STRCONV_H_
#define _KILLME_STRCONV_H_

#include <tchar.h>
#include <string>

/// TODO: comment
#define KILLME_TEXT(txt) _TEXT(txt)

namespace killme
{
    /// TODO: comment
    using tchar = TCHAR;
    using tstring = std::basic_string<tchar, std::char_traits<tchar>, std::allocator<tchar>>;

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
    /*
    std::string toUppers(const std::string& s);
    std::wstring toUppers(const std::wstring& s);
    */

    /** Returns length */
    size_t strlen(const std::string& s);
    size_t strlen(const std::wstring& s);

    /** Compare strings with lowers */
    int strcmpLow(const std::string& a, const std::string& b);
    int strcmpLow(const std::wstring& a, const std::wstring& b);
}

#endif