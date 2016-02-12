#ifndef _KILLME_RESOURCE_H_
#define _KILLME_RESOURCE_H_

#include <memory>
#include <string>
#include <cassert>

namespace killme
{
    /** For managements resources */
    class IsResource
    {
    public:
        virtual ~IsResource() = default;
    };

    namespace detail
    {
        // Call ResourceManager::load() or ResourceManager::unload()
        std::shared_ptr<IsResource> loadResource(const std::string& path);
        void unloadResource(const std::string& path);
    }

    /** The interface of resource */
    template <class T>
    class Resource
    {
    private:
        std::string path_;
        mutable std::weak_ptr<T> resource_;

    public:
        /** Constructs */
        Resource() = default;

        /** Constructs with a path of resource */
        Resource(const std::string& path, const std::weak_ptr<T>& resource)
            : path_(path)
            , resource_(resource)
        {
        }

        /** Accesses the resource. If resource is not loaded, load resource immediately. */
        std::shared_ptr<T> access() const
        {
            if (resource_.expired())
            {
                return load();
            }
            return resource_.lock();
        }

        /** Loads the resource */
        std::shared_ptr<T> load() const
        {
            assert(path_ != "" && "This resource intarface is not bound any resources.");

            const auto cast = std::dynamic_pointer_cast<T>(detail::loadResource(path_));
            assert(cast && "Mismatch the resource type.");
            resource_ = cast;
            return resource_.lock();
        }

        /** Unloads the resource */
        void unload() const
        {
            assert(path_ != "" && "This resource intarface is not bound any resources.");
            detail::unloadResource(path_);
        }
    };
}

#endif
