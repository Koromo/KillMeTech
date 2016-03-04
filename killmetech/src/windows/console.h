#ifndef _KILLME_CONSOLE_H_
#define _KILLME_CONSOLE_H_

#include "../core/string.h"
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
        /** Construct */
        Console();

        /** Allocate console */
        void allocate();

        /** Free console */
        void free();

        /** Read an input until a space character */
        tstring read();

        /** Output a string */
        void write(const tchar* str);
        void writef(const tchar* fmt, ...);
        void writeln(const tchar* str);
        void writefln(const tchar* fmt, ...);
    };

    extern Console console;
}

#endif