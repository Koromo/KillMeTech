#include "console.h"
#include "../core/exception.h"
#include "winsupport.h"
#include <cstdarg>

namespace killme
{
    Console console;

    void Console::allocate()
    {
        enforce<WindowsException>(AllocConsole(), "Failed to allocate the console.");
        inHandle_ = GetStdHandle(STD_INPUT_HANDLE);
        outHandle_ = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    void Console::free()
    {
        FreeConsole();
    }

    tstring Console::read()
    {
        tchar input[1024];
        DWORD numRead;
        ReadConsole(inHandle_, input, 1023, &numRead, NULL);
        input[numRead - 2] = KILLME_T('\0');
        return input;
    }

    void Console::write(const tchar* str)
    {
        WriteConsole(outHandle_, str, static_cast<DWORD>(strlen(str)), NULL, NULL);
    }

    void Console::writef(const tchar* fmt, ...)
    {
        tchar buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);
        Console::write(buffer);
        va_end(args);
    }

    void Console::writeln(const tchar* str)
    {
        write(str);
        write(KILLME_T("\n"));
    }

    void Console::writefln(const tchar* fmt, ...)
    {
        tchar buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);
        Console::writeln(buffer);
        va_end(args);
    }
}