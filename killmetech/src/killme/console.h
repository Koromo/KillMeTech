#ifndef _KILLME_CONSOLE_H_
#define _KILLME_CONSOLE_H_

#include "string.h"
#include <Windows.h>

namespace killme
{
    /** Console interface */
    class Console
    {
    private:
        HANDLE inHandle_;
        HANDLE outHandle_;

    public:
        /** Allocate console */
        Console();

        /** Free console */
        ~Console();

        /** Read input */
        tstring read();

        /** Output string */
        void write(const tchar* str);
        void writef(const tchar* fmt, ...);
        void writeln(const tchar* str);
        void writefln(const tchar* fmt, ...);
    };
}

#endif