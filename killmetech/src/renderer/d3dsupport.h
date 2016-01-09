#ifndef _KILLME_D3DSUPPORT_H_
#define _KILLME_D3DSUPPORT_H_

#include "../core/exception.h"
#include <string>

namespace killme
{
    /** Direct3D relational exception */
    class Direct3DException : public Exception
    {
    public:
        /** Construct with a message */
        explicit Direct3DException(const std::string& msg) : Exception(msg) {}
    };
}

#endif
