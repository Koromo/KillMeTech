#ifndef _KILLME_RESOURCE_H_
#define _KILLME_RESOURCE_H_

#include <memory>
#include <string>
#include <functional>
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
        struct Holder
        {
            virtual std::unique_ptr<Holder> copy() const = 0;
            virtual std::shared_ptr<T> access() const = 0;
            virtual std::shared_ptr<T> load() const = 0;
            virtual void unload() const = 0;
        };

        // For managed resources
        struct ManagedHolder : Holder
        {
            std::string path;
            mutable std::weak_ptr<T> resource;
            
            ManagedHolder(const std::string& p, const std::weak_ptr<T>& r)
                : path(p)
                , resource(r)
            {
            }
            
            std::unique_ptr<Holder> copy() const
            {
                return std::make_unique<ManagedHolder>(path, resource);
            }

            std::shared_ptr<T> access() const
            {
                if (resource.expired())
                {
                    return load();
                }
                return resource.lock();
            }
            
            std::shared_ptr<T> load() const
            {
                assert(path != "" && "This resource intarface is not bound any resources.");

                const auto cast = std::dynamic_pointer_cast<T>(detail::loadResource(path));
                assert(cast && "Mismatch the resource type.");
                resource = cast;
                return resource.lock();
            }

            void unload() const
            {
                assert(path != "" && "This resource intarface is not bound any resources.");
                detail::unloadResource(path);
            }
        };

        // For nonmanaged resources
        struct NonmanagedHolder : Holder
        {
            using Loader = std::function<std::shared_ptr<T>()>;

            mutable Loader loader;
            mutable std::shared_ptr<T> resource;

            NonmanagedHolder(Loader l, const std::shared_ptr<T>& r)
                : loader(l)
                , resource(r)
            {
            }

            std::unique_ptr<Holder> copy() const
            {
                return std::make_unique<NonmanagedHolder>(loader, resource);
            }

            std::shared_ptr<T> access() const
            {
                if (!resource)
                {
                    return load();
                }
                return resource;
            }

            std::shared_ptr<T> load() const
            {
                resource = loader();
                return resource;
            }

            void unload() const
            {
                resource.reset();
            }
        };

        std::unique_ptr<Holder> holder_;

    public:
        /** Constructs */
        Resource() = default;

        /** Constructs as the managed resource */
        Resource(const std::string& path, const std::weak_ptr<T>& resource)
            : holder_(std::make_unique<ManagedHolder>(path, resource))
        {
        }

        /** Constructs as the nonmanaged resource */
        /// TODO: (Loader, Resource) signature order is not compilable
        Resource(const std::shared_ptr<T>& resource, typename NonmanagedHolder::Loader loader)
            : holder_(std::make_unique<NonmanagedHolder>(loader, resource))
        {
        }

        /** Copy constructor */
        Resource(const Resource& lhs)
            : holder_()
        {
            *this = lhs;
        }

        /** Move constructor */
        Resource(Resource&& rhs) = default;

        /** Copy assignment operator */
        Resource& operator=(const Resource& lhs)
        {
            holder_.reset();
            if (lhs.holder_)
            {
                holder_ = lhs.holder_->copy();
            }
            return *this;
        }

        /** Move assignment operator */
        Resource& operator=(Resource&& rhs) = default;

        /** Accesses the resource. If resource is not loaded, load resource immediately. */
        std::shared_ptr<T> access() const
        {
            return holder_->access();
        }

        /** Loads the resource */
        std::shared_ptr<T> load() const
        {
            return holder_->load();
        }

        /** Unloads the resource */
        void unload() const
        {
            return holder_->unload();
        }
    };
}

#endif
