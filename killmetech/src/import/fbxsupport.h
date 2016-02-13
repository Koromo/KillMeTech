#ifndef _KILLME_FBXSUPPORT_H_
#define _KILLME_FBXSUPPORT_H_

#include "../core/exception.h"
#include <memory>

namespace killme
{
    /** The exception of Fbxsdk */
    class FbxException : public Exception
    {
    public:
        /** Constructs with a message */
        explicit FbxException(const std::string& msg) : Exception(msg) {}
    };

    namespace detail
    {
        // Deleter for Fbxsdk objects
        template <class T>
        struct FbxDeleter
        {
            void operator()(T* p)
            {
                p->Destroy();
            }
        };
    }

    // Smart pointers for Fbxsdk objects
    template <class T>
    using FbxUniquePtr = std::unique_ptr<T, detail::FbxDeleter<T>>;

    template <class T>
    using FbxSharedPtr = std::shared_ptr<T>;

    template <class T>
    FbxUniquePtr<T> makeFbxUnique(T* p)
    {
        return FbxUniquePtr<T>(p, detail::FbxDeleter<T>());
    }

    template <class T>
    FbxSharedPtr<T> makeFbxShared(T* p)
    {
        return FbxSharedPtr<T>(p, detail::FbxDeleter<T>());
    }
}

#endif