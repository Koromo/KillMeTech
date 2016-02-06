#ifndef _KILLME_CONSOLE_H_
#define _KILLME_CONSOLE_H_

#include "../core/string.h"
#include <Windows.h>

namespace killme
{
    /** The console interface */
    class Console
    {
    private:
        HANDLE inHandle_;
        HANDLE outHandle_;

    public:
        /** Allocates the console */
        void allocate();

        /** Frees the console */
        void free();

        /** Reads an input until the space character */
        tstring read();

        /** Outputs a string */
        void write(const tchar* str);
        void writef(const tchar* fmt, ...);
        void writeln(const tchar* str);
        void writefln(const tchar* fmt, ...);
    };

    extern Console console;
}

#endif