#ifndef _KILLME_WINSUPPORT_H_
#define _KILLME_WINSUPPORT_H_

#include "../core/exception.h"
#include <string>
#include <memory>

namespace killme
{
    /** Windows platform relational exception */
    class WindowsException : public Exception
    {
    public:
        /** Construct with a message */
        explicit WindowsException(const std::string& msg);
    };

    namespace detail
    {
        // Deleter for COM objects
        template <class T>
        struct ComDeleter
        {
            void operator()(T* p)
            {
                if (p)
                {
                    p->Release();
                }
            }
        };
    }

    /** Smart pointers for COM objects */
    template <class T>
    using ComUniquePtr = std::unique_ptr<T, detail::ComDeleter<T>>;

    template <class T>
    using ComSharedPtr = std::shared_ptr<T>;

    /** Make helpers */
    template <class T>
    ComUniquePtr<T> makeComUnique(T* p)
    {
        return ComUniquePtr<T>(p, detail::ComDeleter<T>());
    }

    template <class T>
    ComSharedPtr<T> makeComShared(T* p)
    {
        return ComSharedPtr<T>(p, detail::ComDeleter<T>());
    }
}

#endif