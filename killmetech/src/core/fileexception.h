#ifndef _KILLME_FILE_H_
#define _KILLME_FILE_H_

#include "exception.h"

namespace killme
{
    /** File relational exception */
    class FileException : public Exception
    {
    public:
        /** Construct with a message */
        explicit FileException(const std::string& msg);
    };
}

#endif