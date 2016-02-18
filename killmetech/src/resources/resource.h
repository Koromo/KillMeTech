#ifndef _KILLME_RESOURCE_H_
#define _KILLME_RESOURCE_H_

#include <memory>
#include <string>
#include <functional>
#include <cassert>

namespace killme
{
    class ResourceManager;

    /** For resource manegement */
    class IsResource
    {
    public:
        virtual ~IsResource() = default;
    };

    namespace detail
    {
        std::shared_ptr<IsResource> callLoad(ResourceManager& msg, const std::string& path);
        void callUnload(ResourceManager& msg, const std::string& path);
    }

    /** Resource accessor */
    /// NOTE: Not support const resources
    template <class T>
    class Resource
    {
    private:
        struct Holder
        {
            virtual std::unique_ptr<Holder> copy() const = 0;
            virtual std::shared_ptr<T> access() const = 0;
            virtual void load() const = 0;
            virtual void unload() const = 0;
        };

        // For managed resources
        struct ManagedHolder : Holder
        {
            ResourceManager& mng;
            std::string path;
            mutable std::weak_ptr<T> resource;
            
            ManagedHolder(ResourceManager& m, const std::string& p, const std::weak_ptr<T>& r)
                : mng(m)
                , path(p)
                , resource(r)
            {
            }
            
            std::unique_ptr<Holder> copy() const
            {
                return std::make_unique<ManagedHolder>(mng, path, resource);
            }

            std::shared_ptr<T> access() const
            {
                if (resource.expired())
                {
                    load();
                }
                return resource.lock();
            }
            
            void load() const
            {
                resource = std::dynamic_pointer_cast<T>(detail::callLoad(mng, path));
                assert(resource.lock() && "Mismatch the resource type.");
            }

            void unload() const
            {
                detail::callUnload(mng, path);
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
                    load();
                }
                return resource;
            }

            void load() const
            {
                resource = loader();
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
        Resource(ResourceManager& msg, const std::string& path, const std::weak_ptr<T>& resource)
            : holder_(std::make_unique<ManagedHolder>(msg, path, resource))
        {
        }

        /** Constructs as the nonmanaged resource */
        Resource(typename NonmanagedHolder::Loader loader, const std::shared_ptr<T>& resource)
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
        void load() const
        {
            holder_->load();
        }

        /** Unloads the resource */
        void unload() const
        {
            return holder_->unload();
        }
    };
}

#endif
